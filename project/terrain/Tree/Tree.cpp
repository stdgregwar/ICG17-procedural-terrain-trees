#include "Tree.h"

#include <glm/gtc/type_ptr.hpp>

using namespace glm;

Tree::Tree(Material &truncMaterial, Material &leafMaterial) :
    mTruncMaterial(truncMaterial),
    mLeafMaterial(leafMaterial),
    mRand(-1,1)
{

}

std::tuple<vec3,vec3,vec3,vec3> bezierBase(const Bezier<vec3>& b, float p, float p2, const vec3& hint) {
    vec3 center = b.curveAtTime(p);
    vec3 next = b.curveAtTime(p2);
    vec3 forw = normalize(next-center);
    vec3 side = cross(forw,vec3(0,1,0));
    vec3 bside = cross(side,forw);
    return tie(center,forw,side,bside);
}


void Tree::addTrunc(const Bezier<vec3> &b, float count, float res, float width) {

    auto& vertices = mTruncVerts;
    auto& indices = mTruncIndices;
    int begin = vertices.size();
    //Bezier<vec3> b({{b,b+normal,b+normal+vec3(0,0,1)*length(normal)}});
    for(float i = 0; i < count; i++) {
        float t = i*0.9999 / count;
        float twidth = width*(1-(i+1)/count);
        vec3 center, forw, side, bside;
        tie(center,forw,side,bside) = bezierBase(b,t,t+1/count,vec3(1,0,0));
        for(float j = 0; j <= res; j++) {
            float ang = M_PI*2*j/res;
            vec3 norm = cos(ang)*side + sin(ang)*bside;
            vec3 point = center + norm*twidth;
            vertices.push_back({
                                   point,
                                   norm,
                                   vec3(0,0,t),
                                   vec2(j/res,t)
                               });

            //Add indices
            if(i != count -1) {
                int offset = begin+ i * (res+1) + j;
                indices.push_back(offset + res + 1);
                indices.push_back(offset + 1);
                indices.push_back(offset + 0);
                indices.push_back(offset + res + 1 +1  );
                indices.push_back(offset + 1);
                indices.push_back(offset + res +1);
            }
        }
    }
}

void Tree::addLeaves(const Bezier<glm::vec3>& b, float count) {
    for(float i = 0.2; i < 1; i+=1/count) {
        vec3 pos = b.curveAtTime(i);
        vec3 jitter = vec3(mRand(mEng),mRand(mEng),mRand(mEng));
        vec3 col = {i,mRand(mEng)*0.5+0.5,mRand(mEng)*0.5+0.5};
        float size = 1 + mRand(mEng)*0.3;
        mLeafVerts.push_back({pos+jitter,col,size});
    }
}

void Tree::build(const glm::vec3& pos, const glm::vec3& normal, float width) {
    //mMaterial.init("v_tree.glsl","f_tree.glsl");

    mEng.seed(pos.x); //Set the seed of the tree, this way we always end up with the same for a specific place

    std::vector<TruncData>& vertices = mTruncVerts;
    std::vector<GLuint>& indices = mTruncIndices;

    float count = 20;
    float res = 12;
    Bezier<vec3> mainTrunc({{pos,pos+normal,pos+normal+vec3(0,0,1)*length(normal)}});
    addTrunc(mainTrunc,count,res,width);

    float bCount = 40;
    for(float i = 0.4; i < 0.9; i+=1/bCount) {
        vec3 center, forw, side, bside;
        float twidth = width*(1-i);
        float lenght = normal.length()*4*(1-i);
        tie(center,forw,side,bside) = bezierBase(mainTrunc,i,i+1/count,vec3(1,0,0));
        float ang = 2*M_PI*mRand(mEng);
        vec3 dir = cos(ang) * side + sin(ang)* bside - forw;
        dir*=lenght;
        Bezier<vec3> branch({{center,center+dir,center+dir+vec3(1,0,0)}});
        addTrunc(branch,4,4,twidth*0.2);
    }

    mTruncMaterial.bind();

    glGenVertexArrays(1, &mVertexArrayId);
    glBindVertexArray(mVertexArrayId);

    // position buffer
    glGenBuffers(1, &mVertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(TruncData),
                 vertices.data(), GL_STATIC_DRAW);

    // vertex indices
    glGenBuffers(1, &mVertexBufferObjectIndex);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVertexBufferObjectIndex);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                 indices.data(), GL_STATIC_DRAW);

    mTruncSize = indices.size();

    {
        // position shader attribute
        GLuint locPosition = mTruncMaterial.attrLocation("position");
        if(locPosition == -1) {
            throw std::runtime_error("failed to bind attrib position");
        }
        glEnableVertexAttribArray(locPosition);
        glVertexAttribPointer(locPosition, 3, GL_FLOAT, DONT_NORMALIZE,
                              sizeof(TruncData), (void*)offsetof(TruncData,pos));

        GLuint locNormal = mTruncMaterial.attrLocation("normal");
        if(locNormal == -1) {
            throw std::runtime_error("failed to bind attrib normal");
        }
        glEnableVertexAttribArray(locNormal);
        glVertexAttribPointer(locNormal, 3, GL_FLOAT, DONT_NORMALIZE,
                              sizeof(TruncData), (void*)offsetof(TruncData,normal));

        GLuint locColor = mTruncMaterial.attrLocation("color");
        if(locColor == -1) {
            throw std::runtime_error("failed to bind attrib color");
        }
        glEnableVertexAttribArray(locColor);
        glVertexAttribPointer(locColor, 3, GL_FLOAT, DONT_NORMALIZE,
                              sizeof(TruncData), (void*)offsetof(TruncData,col));

        GLuint locUV = mTruncMaterial.attrLocation("uv");
        if(locUV == -1) {
            throw std::runtime_error("failed to bind attrib uv");
        }
        glEnableVertexAttribArray(locUV);
        glVertexAttribPointer(locUV, 2, GL_FLOAT, DONT_NORMALIZE,
                              sizeof(TruncData), (void*)offsetof(TruncData,uv));

    }
    glBindVertexArray(0);

    //===================================================================================
    //Leaf buffers
    mLeafMaterial.bind();

    glGenVertexArrays(1, &mLeafVertexArrayId);
    glBindVertexArray(mLeafVertexArrayId);

    // position buffer
    glGenBuffers(1, &mLeafVertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, mLeafVertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, mLeafVerts.size() * sizeof(LeafData),
                 mLeafVerts.data(), GL_STATIC_DRAW);

    {
        // position shader attribute
        GLuint locPosition = mLeafMaterial.attrLocation("position");
        if(locPosition == -1) {
            throw std::runtime_error("failed to bind attrib position");
        }
        glEnableVertexAttribArray(locPosition);
        glVertexAttribPointer(locPosition, 3, GL_FLOAT, DONT_NORMALIZE,
                              sizeof(LeafData), (void*)offsetof(LeafData,pos));

        GLuint locColor = mLeafMaterial.attrLocation("color");
        if(locColor == -1) {
            throw std::runtime_error("failed to bind attrib color");
        }
        glEnableVertexAttribArray(locColor);
        glVertexAttribPointer(locColor, 3, GL_FLOAT, DONT_NORMALIZE,
                              sizeof(LeafData), (void*)offsetof(LeafData,col));
        GLuint locSize = mLeafMaterial.attrLocation("size");
        if(locSize == -1) {
            throw std::runtime_error("failed to bind attrib size");
        }
        glEnableVertexAttribArray(locSize);
        glVertexAttribPointer(locSize, 1, GL_FLOAT, DONT_NORMALIZE,
                              sizeof(LeafData), (void*)offsetof(LeafData,size));
    }
    glBindVertexArray(0);
    mLeafMaterial.bind();
}

void Tree::drawTrunc(const glm::mat4& view, const glm::mat4& proj, Material &mat) {
    glBindVertexArray(mVertexArrayId);
    glUniformMatrix4fv(mat.uniformLocation("VP"), ONE, DONT_TRANSPOSE,glm::value_ptr(proj*view));
    glDrawElements(GL_TRIANGLES, mTruncSize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

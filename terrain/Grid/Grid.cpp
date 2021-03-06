#include "Grid.h"

Grid::Grid(Material& m) : mMaterial(m)
{

}

void Grid::init(int res, bool seams) {
    mRes = res;

    mMaterial.bind();
    // vertex one vertex array
    glGenVertexArrays(1, &mVertexArrayId);
    glBindVertexArray(mVertexArrayId);

    // vertex coordinates and indices
    {
        std::vector<GridVertex> vertices;
        std::vector<GLuint> indices;

        int gridDim = res;

        int start = seams ? -1 : 0;
        int end = seams ? gridDim+1 : gridDim;
        for(int x = start; x <= end; ++x)
        {
            for(int y = start; y <= end; ++y)
            {
                float shift = (x == -1 || y == -1 || y == gridDim+1 || x == gridDim+1) ? -32 : 0;
                glm::vec2 pos = {min(max((float)x/gridDim,0.f),1.f),min(max((float)y/gridDim,0.f),1.f)};
                vertices.push_back({pos,shift});
            }
        }

        if(seams) gridDim+=2;

        for(int x = 0; x < gridDim; ++x)
        {
            for(int y = 0; y < gridDim; ++y)
            {

                int offset = x * (gridDim+1) + y;
                indices.push_back(offset + gridDim + 1);
                indices.push_back(offset + 1);
                indices.push_back(offset + 0);
                indices.push_back(offset + gridDim + 1 + 1);
                indices.push_back(offset + 1);
                indices.push_back(offset + gridDim + 1);


            }
        }

        mNumIndices = indices.size();

        // position buffer
        glGenBuffers(1, &mVertexBufferObjectPosition);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferObjectPosition);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GridVertex),
                     &vertices[0], GL_STATIC_DRAW);

        // vertex indices
        glGenBuffers(1, &mVertexBufferObjectIndex);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVertexBufferObjectIndex);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                     &indices[0], GL_STATIC_DRAW);

        // position shader attribute
        GLuint locPosition = mMaterial.attrLocation("position");
        if(locPosition == -1) {
            throw std::runtime_error("failed to bind attrib position");
        }
        glEnableVertexAttribArray(locPosition);
        glVertexAttribPointer(locPosition, 2, GL_FLOAT, DONT_NORMALIZE,
                              sizeof(GridVertex), (void*)offsetof(GridVertex,pos));

        GLuint locShift = mMaterial.attrLocation("shift");
        if(locShift == -1) {
            throw std::runtime_error("failed to bind attrib shift");
        }
        glEnableVertexAttribArray(locShift);
        glVertexAttribPointer(locShift, 1, GL_FLOAT, DONT_NORMALIZE,
                               sizeof(GridVertex), (void*)offsetof(GridVertex,shift));
    }


    // other uniforms
    mMVPId = mMaterial.uniformLocation("MVP");

    mHeightMapLoc = mMaterial.uniformLocation("height_map");

    glBindVertexArray(0);
    mMaterial.unbind();
}

void Grid::cleanup() {
    glBindVertexArray(0);
    glUseProgram(0);
    glDeleteBuffers(1, &mVertexBufferObjectPosition);
    glDeleteBuffers(1, &mVertexBufferObjectIndex);
    glDeleteVertexArrays(1, &mVertexArrayId);
}

void Grid::draw(float time, const glm::mat4 &model,
          const glm::mat4 &view,
          const glm::mat4 &projection, Material &mat, GLfloat alpha, GLuint heightMap, GLuint texRes) const {
    //mMaterial.bind();
    glBindVertexArray(mVertexArrayId);

    // bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightMap);
    glUniform1i(mHeightMapLoc,0);

    // setup MVP
    glm::mat4 MVP = projection*view*model;
    glUniformMatrix4fv(mat.uniformLocation("MVP"), ONE, DONT_TRANSPOSE, glm::value_ptr(MVP));
    glUniformMatrix4fv(mat.uniformLocation("MV"), ONE, DONT_TRANSPOSE,glm::value_ptr(view*model));
    glUniformMatrix4fv(mat.uniformLocation("V"), ONE, DONT_TRANSPOSE,glm::value_ptr(view));
    glUniformMatrix4fv(mat.uniformLocation("iV"), ONE, DONT_TRANSPOSE,glm::value_ptr(inverse(view)));
    glUniformMatrix4fv(mat.uniformLocation("P"), ONE, DONT_TRANSPOSE,glm::value_ptr(projection));
     glUniformMatrix4fv(mat.uniformLocation("iP"), ONE, DONT_TRANSPOSE,glm::value_ptr(inverse(projection)));
    glUniformMatrix4fv(mat.uniformLocation("VP"), ONE, DONT_TRANSPOSE,glm::value_ptr(projection*view));
    glUniformMatrix4fv(mat.uniformLocation("M"), ONE, DONT_TRANSPOSE,glm::value_ptr(model));
    glUniform1f(mat.uniformLocation("alpha"), alpha);

    // pass the current time stamp to the shader.
    glUniform1f(mat.uniformLocation("time"), time);
    glUniform1f(mat.uniformLocation("res"), texRes == -1 ? mRes : texRes);

    // drawing the grid
    //lPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, mNumIndices, GL_UNSIGNED_INT, 0);
    //glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

    glBindVertexArray(0);
    //mMaterial.unbind();
}

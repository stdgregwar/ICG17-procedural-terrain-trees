#ifndef MATERIAL_H
#define MATERIAL_H

#include "icg_helper.h"
#include "HashCache.h"

struct Texture {
    string name;
    GLuint id;
    GLuint target;
    GLuint no;
    GLuint loc;
};

class Material
{
public:
    Material();
    Material(const string& vshader, const string& fshader);
    void init(const string& vshader, const string& fshader);
    void init(const string& vshader, const string& fshader, const string& gshader);
    GLuint attrLocation(const string& name) const;
    GLuint uniformLocation(const string& name) const;
    GLuint addStippleTex(GLuint no, const string& uname);
    GLuint addTexture(GLuint no, const string& filename, const string& uName, GLuint filter = GL_LINEAR_MIPMAP_LINEAR,GLuint repeat=GL_REPEAT,bool genMipmaps = true);
    GLuint addTexture(GLuint no, const string& filename, const string& uName, GLuint filter = GL_LINEAR_MIPMAP_LINEAR,GLuint repeatS=GL_REPEAT, GLuint repeatT=GL_REPEAT,bool genMipmaps = true);
    GLuint addTexture(GLuint target, GLuint no, const void* data, GLuint format,GLuint type, const vector<size_t>& dim, const string& uName, GLuint filter = GL_LINEAR,GLuint repeatS=GL_REPEAT, GLuint repeatT = GL_REPEAT,bool genMipmaps = false);
    GLuint addTexture(GLuint target, GLuint no, const void* data, GLuint format,GLuint type, const vector<size_t>& dim, const string& uName, GLuint filter = GL_LINEAR,GLuint repeat=GL_REPEAT,bool genMipmaps = false);
    GLuint addTexture(GLuint target, GLuint no,GLuint texId,const string& uName);
    GLuint addCubeTexture(GLuint target, GLuint no, vector<const GLchar*> faces,const string& uName);
    void bind() const;
    void unbind() const;
    ~Material();
private:
    GLuint mProgramId;
    vector<Texture> mTextures;
    mutable rmg::HashCache<string, GLuint> mULocCache;
};

#endif // MATERIAL_H

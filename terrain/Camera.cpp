#include "Chunk.h"
#include "Camera.h"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>


#ifndef M_PI
#define M_PI 3.1415
#endif

using namespace glm;
using namespace std;

Camera::Camera(const vec3 &pos, const vec3 &orientation) : mRotation(orientation), mPosition(pos)
{

}

const glm::mat4& Camera::projection() const {
    return mProjection;
}


const glm::mat4& Camera::view() const {
    return mView;
}

void Camera::setProjection(const glm::mat4& projection) {
    mProjection = projection;
}

glm::vec3 Camera::forward() const {
    return vec3(inverse(mView)*vec4(0,0,-1,0));
}

glm::vec3 Camera::inMap(const glm::vec3& pos, const Chunk& c) {
    const SharedTexture& st = c.hMap();
    if(!st.get()) return pos;

    glClampColor(GL_CLAMP_READ_COLOR, GL_FIXED_ONLY);
    int x; int y;
    vec2 size = c.size();
    vec2 cpos = c.pos();
    //cout << "Cpos " << cpos.x << " " << cpos.y << endl;
    vec2 rcpos = {pos.x-cpos.x,pos.y-cpos.y};
    //cout << "RCpos " << rcpos.x << " " << rcpos.y << endl;
    vec2 texPos = rcpos*float(st->res()) / size;
    //cout << "Tex " << texPos.x << " " << texPos.y << endl;
    float h = st->valAt(texPos.x,texPos.y);
    h = std::max(h,0.f);
    h+=7;
    h = pos.z < h ? h : pos.z;
    return vec3{pos.x,pos.y,h};
}

bool Camera::inFrustum(const glm::vec2& pos, const float &chunkSize) const {
    glm::vec4 mFrustum[6];
    glm::mat4 VP = mProjection*mView;
    for(int i = 0; i < 3; i++) {
        mFrustum[i].x = VP[0][3] + VP[0][i];
        mFrustum[i].y = VP[1][3] + VP[1][i];
        mFrustum[i].z = VP[2][3] + VP[2][i];
        mFrustum[i].w = VP[3][3] + VP[3][i];

        mFrustum[i+1].x = VP[0][3] - VP[0][i];
        mFrustum[i+1].y = VP[1][3] - VP[1][i];
        mFrustum[i+1].z = VP[2][3] - VP[2][i];
        mFrustum[i+1].w = VP[3][3] - VP[3][i];
        mFrustum[i] = glm::normalize(mFrustum[i]);
        mFrustum[i+1] = glm::normalize(mFrustum[i+1]);
    }

    glm::vec3 mins = glm::vec3(pos,-4096);
    glm::vec3 maxs = mins + glm::vec3(chunkSize,chunkSize,8192);
    glm::vec3 vmin, vmax;

    for(int i =0; i < 6; i++) {
        const float w = mFrustum[i].w;
        const glm::vec3 normal = vec3(mFrustum[i]);

        // X axis
        if(mFrustum[i].x > 0) {
            vmin.x = mins.x;
            vmax.x = maxs.x;
        } else {
            vmin.x = maxs.x;
            vmax.x = mins.x;
        }
        // Y axis
        if(mFrustum[i].y > 0) {
            vmin.y = mins.y;
            vmax.y = maxs.y;
        } else {
            vmin.y = maxs.y;
            vmax.y = mins.y;
        }
        // Z axis
        if(mFrustum[i].z > 0) {
            vmin.z = mins.z;
            vmax.z = maxs.z;
        } else {
            vmin.z = maxs.z;
            vmax.z = mins.z;
        }

        if(glm::dot(normal,vmin)   + w > 0){
            return true;
        }
        if(glm::dot(normal,vmax) + w < 0){
            return false;
        }
    }
    return false;
}


glm::vec2 Camera::wPos() const {
    return {mPosition.x,mPosition.y};
}

#version 330


#define amount 2
#define dist 500.f
#define trans 20.f
#define adist 200.f
#define gtiles 10

layout(triangles) in;
layout(triangle_strip, max_vertices=16) out;

in vData {
    vec2 uv;
    vec3 normal_m;
    vec3 w_pos;
    float base_color;
} vertices[];

out vData {
    vec2 uv;
    float alpha;
    vec3 normal;
} vertex;

uniform mat4 VP;
uniform mat4 V;
uniform mat4 iV;
uniform mat4 l_VP;

uniform float time;

#include rand.glsl

vec2 uvFrame(vec2 base, int i) {
    return vec2(float(i)/gtiles + base.x/gtiles,base.y);
}

///Create a billboard quad at given pos
void patchAt(vec3 bpos, vec3 dir) {
    //bpos.z -= 0.5;
    //vec3 side = normalize((iV*vec4(0.1,0,0,0)).xyz)*4;
    vec3 side = normalize(vec3(rand2(bpos.xy),0))*4;
    float ph = rand(bpos.xy);
    vec3 top = dir*3+vec3(rand2(bpos.xy),0)*sin((time+ph)*20*ph)*0.2;
    int i = int((0.5+rand(bpos.xy)*0.5)*gtiles);

    vec3 pos = bpos-side;
    gl_Position = VP * vec4(pos,1.0);
    vertex.uv = uvFrame(vec2(0,0),i);
    EmitVertex();

    pos = bpos+side;
    gl_Position = VP * vec4(pos,1.0);
    vertex.uv = uvFrame(vec2(1,0),i);
    EmitVertex();

    pos = bpos-side+top;
    gl_Position = VP * vec4(pos,1.0);
    vertex.uv = uvFrame(vec2(0,1),i);
    EmitVertex();

    pos = bpos+side+top;
    gl_Position = VP * vec4(pos,1.0);
    vertex.uv = uvFrame(vec2(1,1),i);
    EmitVertex();

    EndPrimitive();
}


void main()
{
    vec3 vPos = (iV*vec4(0,0,0,1)).xyz;
    vec3 cpos = (vertices[0].w_pos + vertices[1].w_pos + vertices[2].w_pos)/3.f;
    vec3 v1 = vertices[1].w_pos - vertices[0].w_pos;
    vec3 v2 = vertices[2].w_pos - vertices[0].w_pos;
    vec3 normal0 = vertices[0].normal_m;
    vec3 normal1 = vertices[1].normal_m;
    vec3 normal2 = vertices[2].normal_m;




    //const int amount = 8;

    vec4 ccpos = VP*vec4(cpos,1);
    vec3 nccpos = ccpos.xyz /ccpos.w;
    const float l = sqrt(2)*1.2;
    if(distance(vec3(0,0,0),nccpos) > l) {
	return;
    }


    const float falloff = amount/dist;
    const float lam = log(float(amount))/log(2.f);
    int tamount = amount;
    float fac = 1.f/tamount;

    v1 *= fac;
    v2 *= fac;

    //v1 = vec3(0.1,0,0);
    //v2 = vec3(0,0.1,0);

    const float baseSize = 2;


    for(int i = 0; i < tamount; i++) {
	for(int j = 0; j < tamount; j++) {
	    if(j+i <= tamount-1) {
		float ifa = float(i)/tamount;
		float jfa = float(j)/tamount;
		vec3 bpos = vertices[0].w_pos + v1 * i + v2 * j;
		vec3 normal = normal0 * (1-jfa) * (1-ifa) + normal1 * ifa + normal2 * jfa;
		normal = normalize(normal);
		vertex.normal = mat3(V)*normal;
		vec3 bnormal = normalize(normal*vec3(1,1,2));
		float normalFac = pow(dot(normal,vec3(0,0,1)),32);
		float bdist = distance(bpos,vPos);
		if(bdist < dist && bpos.z > 0 && bpos.z < 280 && normalFac > 0.1f) {
		    float alpha = clamp((dist-bdist)/(dist-adist),0,1);
		    vertex.alpha = alpha;
		    vec3 randp = vec3(rand2(bpos.xy),0);
		    patchAt(bpos+randp,bnormal*baseSize);
		}
	    }
	}
    }
}

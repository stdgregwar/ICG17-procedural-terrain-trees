#version 330

uniform sampler1D color_map;
uniform sampler2D height_map;
uniform sampler2D grass;
uniform sampler2D cliffs;
uniform sampler2D pebbles;
uniform sampler2D sand;
uniform sampler2D snow;
uniform sampler2D noise;
uniform samplerCube skybox;
uniform float time;
uniform float res;
uniform mat4 MV;
uniform mat4 M;
in vec2 uv;
in vec3 view_dir;
in vec3 normal_mv;
//in vec3 normal_m;
in vec3 light_dir;
in vec3 w_pos;
in float base_color;

out vec4 color;

float height(vec2 p) {
    return texture(height_map,p).r;
}

vec3 fdiff(vec2 p) {
    float d = 0.5f/(res*8);
    float hL = height(p - vec2(d,0));
    float hR = height(p + vec2(d,0));
    float hD = height(p - vec2(0,d));
    float hU = height(p + vec2(0,d));

    vec3 norm;
    // deduce terrain normal
    norm.x = hL - hR;
    norm.y = hD - hU;
    norm.z = 1800*d;
    return normalize(norm);
}

void main() {
    vec3 n = fdiff(uv);
    vec3 normal_m = normalize((M*vec4(n,0)).xyz);
    vec3 normal = normalize((MV*vec4(n,0)).xyz);
    vec3 light = normalize(light_dir);
    vec3 view = normalize(view_dir);

    //>>>>>>>>>> TODO >>>>>>>>>>>
    // TODO 1.2: Phong shading.
    // 1) compute ambient term.
    // 2) compute diffuse term.
    // 3) compute specular term.
    // To avoid GPU bug, remove
    // the code above after
    // implementing Phong shading.
    //<<<<<<<<<< TODO <<<<<<<<<<<
    float diff = clamp(dot(normal,light),0,1);
    vec3 ref = reflect(light,normal);
    float spec = pow(clamp(dot(ref,view),0,1),2);
    float no = texture(noise,w_pos.xy*0.06).r*0.3;
    vec3 b_color = texture(color_map,base_color+no).rgb;

    float dist = pow(clamp(1-distance(b_color,(vec3(0,1,0))),0,1),0.7);
    color = texture(grass,w_pos.xy*0.25)*dist*1.1;

    dist = clamp(1-distance(b_color,(vec3(0,0,1))),0,1);
    color += texture(pebbles,w_pos.xy*0.25)*dist;

    dist = clamp(1-distance(b_color,(vec3(0,1,1))),0,1);
    color += texture(sand,w_pos.xy*0.25)*dist;

    dist = clamp(1-distance(b_color,(vec3(1,0,0))),0,1);
    color += texture(snow,w_pos.xy*0.25)*dist*2;

    float fac = pow(dot(normal_m,vec3(0,0,1)),8);
    vec4 rock = texture(cliffs,w_pos.xy*0.125);
    color = mix(rock,color,fac);

    color *= vec4(0.2,0.3,0.3,1)+vec4(1.1)*diff;//+vec3(1,0.8,0.8)*spec;
    float fog = exp(-0.0004*gl_FragCoord.z/gl_FragCoord.w);
    //color = mix(vec4(0.7, 0.99, 1,1),color,fog);
    color = mix(vec4(1),color,fog);
    color.a = gl_FragCoord.w;
}

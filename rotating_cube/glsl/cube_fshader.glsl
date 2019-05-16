#version 330 core

in float red;
in float green;
in float blue;
out vec3 color;

const vec3 COLOR[6] = vec3[](
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 1.0, 1.0),
    vec3(1.0, 0.0, 0.0),
    vec3(1.0, 0.0, 1.0),
    vec3(1.0, 1.0, 0.0));

void main(){
   color = COLOR[gl_PrimitiveID / 2];
}

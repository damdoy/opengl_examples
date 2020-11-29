#version 400 core

in vec3 vpoint;
in vec2 vtexcoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 uv_tess;

void main(){
   //only calculate the model matrix to have distance between vertex and camera in tess. shader
   gl_Position = model*vec4(vpoint, 1.0);

   uv_tess = vtexcoord;
}

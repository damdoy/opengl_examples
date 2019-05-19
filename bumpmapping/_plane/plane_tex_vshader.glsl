#version 330 core

in vec3 vpoint;
in vec2 vtexcoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 light_position;
uniform vec3 camera_position;

out vec3 frag_position;
out vec2 uv;

void main(){
   gl_Position = projection*view*model*vec4(vpoint, 1.0);

   frag_position = vec3(model*vec4(vpoint, 1.0));
   uv = vtexcoord;
}

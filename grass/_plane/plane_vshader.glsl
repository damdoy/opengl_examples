#version 330 core

in vec3 position;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_position;

uniform mat4 shadow_matrix; //bias*P*V

in vec3 surface_normal;

out vec3 frag_position;
out vec3 frag_normal_transformed;

void main(){
   gl_Position = projection*view*model*vec4(position, 1.0);
}

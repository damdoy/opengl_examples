#version 330 core

in vec3 normal;
in vec3 view_normal;

out vec4 color;

void main(void){
   color = vec4(normal, 1.0);
   color = vec4(view_normal, 1.0);
}

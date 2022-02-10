#version 330 core

in float red;
in float green;
in float blue;
out vec3 color;

void main(){
   color = vec3(red, green, blue);
}

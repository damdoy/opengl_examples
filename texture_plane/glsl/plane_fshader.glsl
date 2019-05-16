#version 330 core

in float red;
in float green;
in float blue;
out vec3 color;

in vec2 uv;
uniform sampler2D tex;

void main(){
   color = 0.5*texture(tex, uv).rgb + 0.5*vec3(red, green, blue);
}

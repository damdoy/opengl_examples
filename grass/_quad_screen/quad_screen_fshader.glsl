#version 330 core
uniform sampler2D tex;
//uniform sampler2DMS tex;
uniform float tex_width;
uniform float tex_height;
in vec2 uv;
out vec3 color;

void main() {

   color = texture(tex, uv).rgb;
}

#version 330 core
uniform sampler2D tex;
uniform sampler2D ao_tex;
//uniform sampler2DMS tex;
uniform float tex_width;
uniform float tex_height;
in vec2 uv;
out vec4 color;

uniform uint effect_select;

void main() {

   vec3 color_out = vec3(0.5, 1.0, 0.0);

   if(effect_select == 0u){ //normal
      color_out = texture(tex, uv).rgb;//*get_ao_blur(uv); (ao_blur caused some glitches)
   }

   color = vec4(color_out, 1.0);
}

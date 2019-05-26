#version 330 core

uniform vec3 camera_position;
uniform vec3 light_position;

uniform sampler2D shadow_buffer_tex;
uniform sampler2D tex_wind;

out vec3 color;

in vec2 frag_uv;
in vec2 global_uv;

void main(){

   //get random value from the noise texture
   float rand_val = mod((global_uv.x*global_uv.x)+(0.13-global_uv.x*global_uv.y), 0.01)/0.01;

   float dist_to_centre = pow(distance(frag_uv, vec2(0.5, 0.5)), 2);

   float dist_to_ground = clamp((1.0-frag_uv.y)*1, 0.0, 1.0);

   color = vec3( (0.1+rand_val/6.0+dist_to_centre)*dist_to_ground, (0.3+dist_to_centre)*dist_to_ground, (0.0+dist_to_centre)*dist_to_ground);
}

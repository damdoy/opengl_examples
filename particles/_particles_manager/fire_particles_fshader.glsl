#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 light_position;

in vec2 frag_uv;
in float frag_life;
in float frag_age;

out vec4 color;

vec3 yellow = vec3(1, 1, 0);
vec3 red = vec3(1, 0, 0);
vec3 gray = vec3(0.5, 0.5, 0.5);

void main(){

   vec2 transf_frag_uv = (frag_uv-0.5)*2;

   if(frag_life == 0){
      discard;
   }

   //round particle
   if(dot(transf_frag_uv, transf_frag_uv) > 1){
      discard;
   }

   if(frag_age < 0.5) //make a bright yellow when particle is young
   {
      color = vec4(mix(yellow, red, frag_age/0.5), 1);
   }
   else{ //then progressivly get gray
      float fraction = (frag_age-0.5)/3;
      fraction = clamp(fraction, 0, 1);
      float transparency = 1-(frag_age-2)/4;
      transparency = clamp(transparency, 0, 1);
      color = vec4(mix(red, gray, fraction), transparency);
   }
}

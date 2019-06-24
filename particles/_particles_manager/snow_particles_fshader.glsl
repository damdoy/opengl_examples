#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 light_position;

in vec2 frag_uv;
in float frag_life;
in float frag_age;

out vec4 color;

void main(){

   vec2 transf_frag_uv = (frag_uv-0.5)*2;

   if(frag_life == 0){ //don't draw this particle if it is dead
      discard;
   }

   //round particle
   if(dot(transf_frag_uv, transf_frag_uv) > 1){
      discard;
   }

   //white snow
   color = vec4(0.85, 0.85, 0.90, 0.5);
}

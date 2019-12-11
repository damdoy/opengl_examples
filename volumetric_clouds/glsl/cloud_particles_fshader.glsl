#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 light_colour;
uniform vec3 shadow_colour;

uniform vec3 light_position;
uniform sampler2D tex_noise_2d;

in vec2 frag_uv;
in float frag_colour;
in float random;
flat in uint frag_active;

out vec4 color;

void main(){

   if(frag_active == 0u){
      discard;
   }

   vec2 transf_frag_uv = (frag_uv-0.5)*2;

   float dist_to_centre = dot(transf_frag_uv, transf_frag_uv);
   float cloud = 1-dist_to_centre;

   // round particle
   if(dist_to_centre > 1){
      discard;
   }

   color = vec4(frag_colour, frag_colour, frag_colour, 0.5*cloud);

   //alpha transparency from a noise texture, at random position
   float alpha = (texture(tex_noise_2d, (frag_uv)*0.25+random*0.75).r+0.5)*(1-pow(dist_to_centre,2));
   // float alpha = 1;

   //if frag colour == 1 will take light_colour
   vec3 final_rgb = mix(shadow_colour, light_colour, frag_colour);

   color = vec4(final_rgb, alpha);
}

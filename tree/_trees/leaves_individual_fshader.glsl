#version 330 core

in vec3 frag_position;

uniform vec3 camera_position;
uniform vec3 light_position;

in vec2 uv_frag;

uniform vec3 sun_col;

out vec4 color;
in float frag_diffuse_light;

in float frag_relative_ground_pos;

void main(){

   float distance_to_middle = distance(uv_frag, vec2(0.5, 0.5));

   if(distance_to_middle > 0.4 ){
      discard;
   }

   float rand_val = sin(frag_position.x*frag_position.z/10)+cos( (0.13-frag_position.x*frag_position.y) / 10 );

   color.a = 1.0;

   float lum = 1.0*frag_diffuse_light;

   float light_intensity = (frag_relative_ground_pos+0.6)*1.5;
   light_intensity = clamp(light_intensity, 0.0, 1.0);
   float reverse_dist_to_middle = 1.0-distance_to_middle;

   // color.rgb = lum*sun_col;
   color.rgb = vec3(0.1+rand_val/6, 0.4, 0.05)*light_intensity*reverse_dist_to_middle;
   // color.rgb = vec3(distance_to_middle);
}

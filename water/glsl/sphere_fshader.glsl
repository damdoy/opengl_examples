#version 330 core

in vec3 frag_surface_normal_color;

in vec3 frag_normal_transformed;
in vec3 frag_position;
in float frag_spot_range;
in float frag_spot_pow;

uniform sampler2D shadow_buffer_tex;

uniform vec3 camera_position;
uniform vec3 light_position;
uniform uint lighting_mode;
uniform bool activate_specular;
uniform vec3 spot_direction;
uniform bool activate_spot;

out vec4 color;

in vec4 shadow_coord;

uniform uint shadow_mapping_effect;
uniform uint shadow_buffer_tex_size;

void main(){
   if(lighting_mode == 0u || lighting_mode == 1u){ //surface or vertex shading
      color = vec4(frag_surface_normal_color, 1.0);
   }
   else{
      vec3 light_dir = normalize(light_position-frag_position);
      float diffuse_light = 0.0;
      float spec_light = 0.0;

      if(activate_specular == true){
         vec3 reflexion = 2*frag_normal_transformed*dot(frag_normal_transformed, light_dir)-light_dir;
         reflexion = normalize(reflexion);
         vec3 view_dir = normalize(camera_position-frag_position);

         spec_light = pow(max(dot(reflexion, view_dir), 0.0), 128);
         spec_light = clamp(spec_light, 0.0, 1.0);
      }

      diffuse_light = dot(frag_normal_transformed, light_dir);

      float lum = 0.8*diffuse_light+spec_light;
      lum = lum;

      color = vec4(lum, lum, lum, 1.0);
   }
}

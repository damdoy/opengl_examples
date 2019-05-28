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

uniform vec3 sun_dir;
uniform vec3 sun_col;

out vec4 color;

in vec4 shadow_coord;

uniform uint shadow_mapping_effect;
uniform uint shadow_buffer_tex_size;

float get_diffuse_strength(vec3 light_dir, vec3 normal){
   return clamp(dot(normal, light_dir), 0.0, 1.0);
}

float get_specular_strength(vec3 light_dir, vec3 normal, vec3 cam_pos, vec3 frag_pos){
   float spec_light = 0.0;

   vec3 reflexion = 2*normal*dot(normal, light_dir)-light_dir;
   reflexion = normalize(reflexion);
   vec3 view_dir = normalize(cam_pos-frag_pos);

   spec_light = pow(max(dot(reflexion, view_dir), 0.0), 128);
   spec_light = clamp(spec_light, 0.0, 1.0);
   return spec_light;
}

void main(){
   if(lighting_mode == 0u || lighting_mode == 1u){ //surface or vertex shading
      color = vec4(frag_surface_normal_color, 1.0);
   }
   else{
      vec3 light_dir = normalize(light_position-frag_position);
      float light_dist = distance(light_position, frag_position);
      float diffuse_light = 0.0;
      float spec_light = 0.0;

      float diffuse_sun = 0.0;
      float spec_sun = 0.0;

      diffuse_light = get_diffuse_strength(light_dir, frag_normal_transformed);
      diffuse_sun = get_diffuse_strength(-sun_dir, frag_normal_transformed);

      if(activate_specular == true){
         spec_light = get_specular_strength(light_dir, frag_normal_transformed, camera_position, frag_position);
         spec_sun = get_specular_strength(-sun_dir, frag_normal_transformed, camera_position, frag_position);
      }

      float spot_brightness = 1.0;

      if(activate_spot == true){
         vec3 spot_direction_norm = normalize(spot_direction);
         float cos_spot = dot(-spot_direction_norm, light_dir);

         if(cos_spot > frag_spot_range){
            spot_brightness = 1.0;
         }
         else{
            spot_brightness = pow(cos_spot+(1.0-frag_spot_range), frag_spot_pow);
         }
      }

      float lum_light = (0.8*diffuse_light+spec_light)*1/(light_dist/8);
      lum_light = spot_brightness*lum_light;
      vec3 light_strength = vec3(lum_light, lum_light, lum_light);

      float lum_sun = (0.8*diffuse_sun+spec_sun);
      vec3 sun_strength = sun_col*lum_sun;
      //color = vec3(texture( shadow_buffer_tex, shadow_coord.xy/shadow_coord.w ).x);
      color = vec4(light_strength+sun_strength, 1.0);
   }
}

#version 330 core

in vec3 frag_surface_normal_color;

in vec3 frag_normal_transformed;
in vec3 frag_position;
in float frag_spot_range;
in float frag_spot_pow;

uniform vec3 camera_position;
uniform vec3 light_position;
uniform uint lighting_mode;
uniform bool activate_specular;
uniform vec3 spot_direction;
uniform bool activate_spot;

out vec3 color;

void main(){
   if(lighting_mode == 0u || lighting_mode == 1u){ //surface or vertex shading => take values calculated in vertex shader
      color = frag_surface_normal_color;
   }
   else{
      vec3 light_dir = normalize(light_position-frag_position);
      float diffuse_light = 0.0;
      float spec_light = 0.0;

      //specular is the bright spot, this is a classical calculation for it
      if(activate_specular == true){
         vec3 reflexion = 2*frag_normal_transformed*dot(frag_normal_transformed, light_dir)-light_dir;
         reflexion = normalize(reflexion);
         vec3 view_dir = normalize(camera_position-frag_position);

         reflexion = clamp(reflexion, 0.0, 1.0);
         spec_light = pow(max(dot(reflexion, view_dir), 0.0), 64);
         spec_light = clamp(spec_light, 0.0, 1.0);
      }

      float spot_brightness = 1.0;

      if(activate_spot == true){
         vec3 spot_direction_norm = normalize(spot_direction);
         float cos_spot = dot(-spot_direction_norm, light_dir);

         if(cos_spot > frag_spot_range){
            spot_brightness = 1.0;
         }
         else{
            //spot brightness goes down fast if out of it
            spot_brightness = pow(cos_spot+(1.0-frag_spot_range), frag_spot_pow);
         }
      }

      diffuse_light = dot(frag_normal_transformed, light_dir);

      float lum = 0.8*diffuse_light+spec_light;
      lum = spot_brightness*lum;

      color = vec3(lum, lum, lum);
   }


}

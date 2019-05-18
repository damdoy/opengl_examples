#version 330 core

in float red;
in float green;
in float blue;
out vec3 color;

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

void main(){

   if(lighting_mode == 0u || lighting_mode == 1u){ //surface or vertex shading => take values calculated in vertex shader
      color = frag_surface_normal_color*vec3(red, green, blue);
   }
   else{
      vec3 light_dir = normalize(light_position-frag_position);
      float diffuse_light = 0.0;

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

      diffuse_light = dot(frag_normal_transformed, light_dir);

      float lum = 0.8*diffuse_light;
      lum = spot_brightness*lum;

      color = lum*vec3(red, green, blue);
   }
}

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

out vec3 color;

in vec4 shadow_coord;

uniform uint shadow_mapping_effect;
uniform uint shadow_buffer_tex_size;

float get_shadow_val(float offset_x, float offset_y);

float get_shadow_val_lerp(float offset_x, float offset_y);

float get_shadow_val_pcf(bool lerp, int size);
float get_shadow_val_pcf_optim(bool lerp, int size);

const vec3 COLOR[6] = vec3[](
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 1.0, 1.0),
    vec3(1.0, 0.0, 0.0),
    vec3(1.0, 0.0, 1.0),
    vec3(1.0, 1.0, 0.0));

void main(){
   //color = COLOR[gl_PrimitiveID%6];
   if(lighting_mode == 0u || lighting_mode == 1u){ //surface or vertex shading
      color = frag_surface_normal_color;
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

      float shadow = 1;

      if(shadow_mapping_effect == 1u){
         shadow = get_shadow_val(0.0, 0.0);
      }
      else if(shadow_mapping_effect == 2u){ //interp
         shadow = get_shadow_val_lerp(0.0, 0.0);
      }
      else if(shadow_mapping_effect == 3u){
         shadow = get_shadow_val_pcf(false, 1);
      }
      else if(shadow_mapping_effect == 4u){
         shadow = get_shadow_val_pcf(true, 1);
      }
      else if(shadow_mapping_effect == 5u){
         shadow = get_shadow_val_pcf(true, 3);
      }
      else if(shadow_mapping_effect == 6u){
         shadow = get_shadow_val_pcf_optim(true, 1);
      }

      diffuse_light = dot(frag_normal_transformed, light_dir);

      float lum = 0.8*diffuse_light+spec_light;
      lum = spot_brightness*lum*shadow;

      //color = vec3(texture( shadow_buffer_tex, shadow_coord.xy/shadow_coord.w ).x);
      color = vec3(lum, lum, lum);
   }
}
float get_shadow_val_pcf(bool lerp, int size){
   float ret_val = 0.0;
   float size_increment = 1.0/float(shadow_buffer_tex_size);
   int counter_avg = 0;

   for(int i = -size; i <= size; i++){
      for(int j = -size; j <= size; j++){
         if(lerp){
            ret_val += get_shadow_val_lerp(i*size_increment, j*size_increment);
         }
         else{
            ret_val += get_shadow_val(i*size_increment, j*size_increment);
         }
         counter_avg++;
      }
   }

   ret_val /= counter_avg;

   return ret_val;
}
float get_shadow_val_pcf_optim(bool lerp, int size){
   float ret_val = 0.0;
   float size_increment = 1.0/float(shadow_buffer_tex_size);
   int counter_avg = 0;

   for(int i = -size; i <= size; i++){
      for(int j = -size; j <= size; j++){
         if(lerp){
            ret_val += get_shadow_val_lerp(i*size_increment*2, j*size_increment*2);
         }
         else{
            ret_val += get_shadow_val(i*size_increment*2, j*size_increment*2);
         }
         counter_avg++;
      }
   }

   ret_val /= counter_avg;

   return ret_val;
}

float get_shadow_val_lerp(float offset_x, float offset_y){
   float val = 0.0;
   float size_increment = 1.0/float(shadow_buffer_tex_size);
   vec2 shadow_tex_position = (shadow_coord.xy/shadow_coord.w) + vec2(offset_x, offset_y);
   //vec2 frac_parts = mod(shadow_tex_position*shadow_buffer_tex_size, 1.0);
   vec2 frac_parts = shadow_tex_position*shadow_buffer_tex_size-floor(shadow_tex_position*shadow_buffer_tex_size);

   float shadow00 = get_shadow_val(0.0+offset_x, 0.0+offset_y);
   float shadow01 = get_shadow_val(0.0+offset_x, size_increment+offset_y);
   float shadow10 = get_shadow_val(size_increment+offset_x, 0.0+offset_y);
   float shadow11 = get_shadow_val(size_increment+offset_x, size_increment+offset_y);

   float shadow0 = (1.0-frac_parts.x)*shadow00+frac_parts.x*shadow10;
   float shadow1 = (1.0-frac_parts.x)*shadow01+frac_parts.x*shadow11;

   val = (1.0-frac_parts.y)*shadow0+frac_parts.y*shadow1;

   return val;
}

float get_shadow_val(float offset_x, float offset_y){

   float shadow = 1.0;
   vec2 offset = vec2(offset_x, offset_y);

   float BIAS = 0.003;

   if ( texture( shadow_buffer_tex, (shadow_coord.xy/shadow_coord.w)+offset ).x  <  shadow_coord.z/shadow_coord.w-BIAS){
       shadow = 0.0;
   }
   return shadow;
}

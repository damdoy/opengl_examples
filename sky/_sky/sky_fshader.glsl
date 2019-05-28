#version 330 core

in vec3 frag_normal_transformed;
in vec3 frag_position;

uniform vec3 camera_position;
uniform vec3 light_position;
uniform uint lighting_mode;
uniform bool activate_specular;
uniform vec3 spot_direction;
uniform bool activate_spot;
uniform float sky_size;

in vec3 sun_dir_corr;

out vec4 color;

const vec3 night_colour = vec3(0.0, 0.0, 0.5);
const vec3 day_colour = vec3(0.5, 0.5, 1.0);
const vec3 red_sky = vec3(0.8, 0.3, 0.1);

void main(){
   vec3 sky_colour = vec3(0.0, 0.0, 0.0);

   sky_colour = night_colour;

   //how much the sun is up or at sunset
   float up_sun = dot(sun_dir_corr, vec3(0.0, -1.0, 0.0));
   float evening_sun = dot(sun_dir_corr, vec3(-1.0, 0.0, 0.0));

   if( up_sun > -0.25){

      float day_quantity = clamp((up_sun+0.25)*3, 0.0, 1.0);

      sky_colour = day_quantity*day_colour+(1.0-day_quantity)*night_colour;
   }

   if(evening_sun > 0.8){

      float evening_quantity = (evening_sun-0.8)*(1/0.2);

      sky_colour = (evening_quantity)*red_sky+(1.0-evening_quantity)*sky_colour;
   }

   //draws the sun
   vec3 pos_sun = (-sun_dir_corr)*sky_size + sun_dir_corr*8;
   vec3 frag_position_comp = frag_position-pos_sun;
   if(dot(frag_position_comp, sun_dir_corr) < 0){
      sky_colour = vec3(1.0, 1.0, 1.0);
   }

   color.a = 1.0;
   color.rgb = sky_colour;
}

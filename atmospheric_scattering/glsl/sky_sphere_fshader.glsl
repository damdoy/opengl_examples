#version 330 core

in vec3 frag_surface_normal_color;

in vec3 frag_normal_transformed;
in vec3 frag_pos;

uniform vec3 camera_position;
uniform vec3 light_position;
uniform vec3 sun_dir;

uniform vec3 atm_centre;
uniform float planet_radius;
uniform float atm_radius;

uniform sampler2D tex_t_func;

out vec4 color;

//mean height of the atm (estimate of a exp)
const float H_0 = 0.25;

//from a position on the atmosphere (end_pos) and a vector,
//find the other point intersecting with the atmosphere
vec3 get_pt_on_atm(vec3 end_pos, vec3 start_dir){

   //find angle between vector from centre to end (on edge) and the vector toward starting point
   float angle_centre_end = acos(dot(normalize(start_dir), normalize(atm_centre-end_pos)));

   //use trig to know the distance to other point
   float dist_to_other_edge = 2*cos(angle_centre_end)*atm_radius;

   return end_pos+start_dir*dist_to_other_edge;
}

//t_func as used in the lookup table, can be used if we want to kill perf
vec3 t_func_simpler(float height_norm, float angle_norm){
   const int nb_integrations = 10;

   float real_height = planet_radius+height_norm*(atm_radius-planet_radius);
   float real_angle = 3.1415*angle_norm;

   float h_sq = real_height*real_height;

   //formula to fin number of time(ratio) we should multiply {sin(angle), cos(angle)} to be on the circle
   //formula from wolfram alpha => r = sqrt( (0+xsin(angle))^2+(h+xcos(angle))^2) solve for x
   //assume centre of atmosphere is 0,0
   float ratio = 1.0/2.0*(sqrt(2)*sqrt(h_sq*cos(2*real_angle)-h_sq+2*atm_radius*atm_radius)-2*real_height*cos(real_angle) );
   // float pt_on_circle[2] = {ratio*sin(real_angle), real_height+ratio*cos(real_angle)};
   float distance_to_end = sqrt(pow(ratio*sin(real_angle), 2)+pow(ratio*cos(real_angle), 2) );
   float scattered = 0.0f;

   float increment = distance_to_end/nb_integrations;
   vec2 current_pos = vec2(0, real_height);

   for(int i = 0; i < nb_integrations; i++)
   {
      //assume centre of atmosphere is 0
      float distance_to_centre = sqrt(pow(current_pos[0]-0, 2)+pow(current_pos[1]-0, 2));
      float atm_density = (distance_to_centre-planet_radius)/(atm_radius-planet_radius);
      if(atm_density < 0){
         atm_density = 0;
      }
      if(atm_density > 1){
         atm_density = 1;
      }

      scattered += exp(-atm_density/H_0);
      current_pos += vec2(increment*sin(real_angle), increment*cos(real_angle));
   }

   scattered /= nb_integrations;

   scattered *= distance_to_end/64;
   scattered *= 4*3.1415;

   return clamp(vec3(scattered*0.1, scattered*0.2, scattered*0.4), 0, 8);
}

//f func as stated in https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter16.html
float f_func(float theta, float g){
   return 3*(1-g*g)/(2*(2+g*g))*(1+cos(theta)*cos(theta))/pow(1+g*g-2*g*cos(theta), 3/2);
}

vec3 iv_func(vec3 pt_start, vec3 pt_end){

   const int nb_integrations = 10;

   vec3 ret_val = vec3(0.4, 0.6, 1); //in scattering, blue
   float angle_sun = acos(dot(normalize(pt_end-pt_start), normalize(-sun_dir)));
   float rayleigh_f_func = f_func(angle_sun, 0);
   float mie_f_func = f_func(angle_sun, -0.98);
   ret_val *= (rayleigh_f_func+mie_f_func);

   //direction toward the camera from point on the atm edge
   vec3 start_dir = normalize(pt_start-pt_end);

   bool cam_in_atm = true;
   if(distance(pt_start, atm_centre) > atm_radius){ //start point (cam) out of atmosphere, get point on atmosphere surface
      pt_start = get_pt_on_atm(pt_end, start_dir); //should on the atm surface
      cam_in_atm = false;
   }

   //resize start dir so that (pt_end+nb_integrations*start_dir) will get us to pt start
   start_dir = start_dir*distance(pt_start, pt_end)/nb_integrations;

   //integral part
   vec3 cur_pos = pt_end;
   vec3 scattered = vec3(0.0f,0,0);
   for(int i = 0; i < nb_integrations; i++)
   {
      float atm_density = (distance(cur_pos, atm_centre)-planet_radius)/(atm_radius-planet_radius);
      atm_density = clamp(atm_density, 0, 1); //shouldn't be, but make sure that we don't go out of atm

      //angles needed for the lookup tables
      float angle_sun = acos(dot(normalize(cur_pos-atm_centre), normalize(-sun_dir)));
      float angle_sun_norm = 1-abs(angle_sun/3.1415);

      float angle_start = acos(dot(normalize(cur_pos-atm_centre), normalize(pt_start-cur_pos)));
      float angle_start_norm = 1-abs(angle_start/3.1415);

      // vec3 t_val_optical_depth = t_func_simpler(atm_density, angle_start_norm);
      vec3 t_val_optical_depth = texture(tex_t_func, vec2(atm_density, angle_start_norm)).rgb;

      float atm_density_cam = (distance(pt_start, atm_centre)-planet_radius)/(atm_radius-planet_radius);
      atm_density_cam = clamp(atm_density_cam, 0, 1);

      //trick as stated in https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter16.html 16.3
      //find optical depth to the edge of atm, then remove the remaining part (from camera to edge)
      angle_start = acos(dot(normalize(cur_pos-atm_centre), normalize(pt_start-cur_pos)));
      angle_start_norm = 1-abs(angle_start/3.1415);

      // t_val_optical_depth -= t_func_simpler(atm_density_cam, angle_start_norm);
      t_val_optical_depth -= texture(tex_t_func, vec2(atm_density_cam, angle_start_norm)).rgb;

      // scattered += exp(-atm_density/H_0)*exp(-t_func_simpler(atm_density, angle_sun_norm)-t_val_optical_depth);
      scattered += exp(-atm_density/H_0)*exp(-texture(tex_t_func, vec2(atm_density, angle_sun_norm)).rgb-t_val_optical_depth);
      cur_pos += start_dir;
   }

   scattered /= nb_integrations;

   //divide distance by parameter for the strength of the effect
   scattered *= distance(pt_start, pt_end)/8;

   return ret_val*scattered;
}

void main(){

   vec3 light_dir = sun_dir;
   vec3 scattering = iv_func(camera_position, frag_pos);

   color = vec4(scattering, 1.0);

}

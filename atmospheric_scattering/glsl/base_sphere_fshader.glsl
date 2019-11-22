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

//from a position which will be on the planet (end_pos) and a direction toward the camera
//use trig to find the intersection with the amospheresphere edge
vec3 get_pt_on_atm_simpler(vec3 end_pos, vec3 start_dir){

   float angle_centre_end = acos(dot(normalize(start_dir), normalize(end_pos-atm_centre)));

   float dist_to_other_edge = (atm_radius-planet_radius)/cos(angle_centre_end);

   return end_pos+normalize(start_dir)*dist_to_other_edge;
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

vec3 t_func(vec3 pt_start, vec3 pt_end){
   const int nb_integrations = 5;
   vec3 start_dir = normalize(pt_end-pt_start);

   // if(set_start_on_atm){
   //    pt_start = get_pt_on_atm(pt_end, start_dir); //should be out of atm
   // }

   vec3 cur_pos = pt_end;

   float scattered = 0.0f;
   int atm_count = 0;

   start_dir = start_dir*distance(pt_start, pt_end)/nb_integrations;

   for(int i = 0; i < nb_integrations; i++)
   {

      //over the increment, the ray intersect with the planet => cancel it
      // if(distance(cur_pos, atm_centre) < planet_radius){
      //    return vec3(4*3.1415, 4*3.1415, 4*3.1415); //everything scattered
      // }

      if(distance(cur_pos, atm_centre) < atm_radius){

         //want to be sure that we are not going behind the start point
         // float pos = dot(pt_start-pt_end, pt_start-cur_pos);
         // if(pos > 0){
            float atm_density = (distance(cur_pos, atm_centre)-planet_radius)/(atm_radius-planet_radius);
            atm_density = clamp(atm_density, 0, 1);
            // scattered += 1;
            scattered += exp(-atm_density/H_0);
            atm_count++;
         // }
      }
      cur_pos -= start_dir;
   }

   // if(atm_count > 0){
   //    scattered /= atm_count;
   // }

   scattered /= nb_integrations;

   scattered *= distance(pt_start, pt_end)/32;
   scattered *= 4*3.1415;

   // return vec3(scattered*0, scattered*0, scattered*0);
   // return vec3(scattered*0, scattered*0.5, scattered*1);
   return clamp(vec3(scattered*0.1, scattered*0.2, scattered*0.4), 0, 4);

}

//f func as stated in https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter16.html
float f_func(float theta, float g){
   return 3*(1-g*g)/(2*(2+g*g))*(1+cos(theta)*cos(theta))/pow(1+g*g-2*g*cos(theta), 3/2);
}

vec3 iv_func(vec3 pt_start, vec3 pt_end){

   const int nb_integrations = 10;

   vec3 ret_val = vec3(0.4, 0.6, 1); //in scattering, blue?
   float angle_sun = acos(dot(normalize(pt_end-pt_start), normalize(-sun_dir)));
   // don't really care about mie for the surface atmosphere on the planet
   // ret_val *= f_func(angle_sun, -0.99); //g=-0.99 mie
   ret_val *= f_func(angle_sun, 0); //g=0 rayleigh


   //direction toward the camera from point on the atm edge
   vec3 start_dir = normalize(pt_start-pt_end);

   bool cam_in_atm = false;
   if(distance(pt_start, atm_centre) > atm_radius){ //start point (cam) out of atmosphere, get point on atmosphere surface
      pt_start = get_pt_on_atm_simpler(pt_end, start_dir); //should be out of atm
      cam_in_atm = true;
   }

   //resize start dir so that (pt_end+nb_integrations*start_dir) will get us to pt start
   start_dir = start_dir*distance(pt_start, pt_end)/nb_integrations;

   //integral part
   vec3 cur_pos = pt_end;
   vec3 scattered = vec3(0.0f,0,0);
   for(int i = 0; i < nb_integrations; i++)
   {
      float atm_density = (distance(cur_pos, atm_centre)-planet_radius)/(atm_radius-planet_radius);
      atm_density = clamp(atm_density, 0, 1);

      //angles needed for the lookup tables
      float angle_sun = acos(dot(normalize(cur_pos-atm_centre), normalize(-sun_dir)));
      float angle_sun_norm = 1-abs(angle_sun/3.1415);

      float angle_start = acos(dot(normalize(cur_pos-atm_centre), normalize(pt_start-cur_pos)));
      float angle_start_norm = 1-abs(angle_start/3.1415);

      // vec3 t_val_optical_depth = t_func_simpler(atm_density, angle_start_norm);
      vec3 t_val_optical_depth = texture(tex_t_func, vec2(atm_density, angle_start_norm)).rgb;

      float atm_density_cam = (distance(pt_start, atm_centre)-planet_radius)/(atm_radius-planet_radius);
      atm_density_cam = clamp(atm_density_cam, 0, 1);

      angle_start = acos(dot(normalize(cur_pos-atm_centre), normalize(pt_start-cur_pos)));
      angle_start_norm = 1-abs(angle_start/3.1415);

      //this *should* be used, but makes the atmosphere a bit "much"
      // t_val_optical_depth -= t_func_simpler(atm_density_cam, angle_start_norm);
      // t_val_optical_depth -= texture(tex_t_func, vec2(atm_density_cam, angle_start_norm)).rgb;

      // scattered += exp(-atm_density/H_0)*exp(-t_func_simpler(atm_density, angle_sun_norm)-t_func(pt_start, cur_pos));
      scattered += exp(-atm_density/H_0)*exp(-texture(tex_t_func, vec2(atm_density, angle_sun_norm)).rgb-t_val_optical_depth);
      cur_pos += start_dir;
   }

   scattered /= nb_integrations;
   //distance division is factor for this parameter
   //dividing more will lessen the effect of an atmosphere on the planet
   //too much will make it too bright
   scattered *= distance(pt_start, pt_end)/2;


   return ret_val*scattered;
}

//really simple noise
float noise(vec3 p){
    return (sin(p.x*0.12)*sin(p.y*0.16)*sin(p.z*0.20))+(cos(p.x*0.4)*cos(p.y*0.2)*cos(p.z*0.14));
}


void main(){
   vec3 light_dir = sun_dir;
   float diffuse_light = 0.0;

   diffuse_light = dot(frag_normal_transformed, light_dir);

   float lum = 0.8*diffuse_light;

   vec3 cam_pos = camera_position;

   if(distance(camera_position, atm_centre) > atm_radius){
      cam_pos = get_pt_on_atm_simpler(frag_pos, normalize(camera_position-frag_pos));
   }

   vec3 grass = vec3(0.4, 0.8, 0.1);
   vec3 water = vec3(0.1, 0.3, 0.6);
   vec3 ice = vec3(0.7, 0.7, 0.7);

   float pos_value = noise(frag_pos);

   //use a simplistic noise function, to generate islands
   vec3 ground_colour;
   if(pos_value > 0.4){
      float relative_val = 1-(pos_value-0.4)/0.6;
      ground_colour = relative_val*grass+(1-relative_val)*ice;
   }
   else{
      ground_colour = water;
   }

   //find density and angle with the sun of vector from planet surface to camera
   //to use lookup table
   float atm_density = (distance(cam_pos, atm_centre)-planet_radius)/(atm_radius-planet_radius);
   atm_density = clamp(atm_density, 0, 1);

   float angle_sun = acos(dot(normalize(frag_pos-atm_centre), normalize(-sun_dir)));
   float angle_sun_norm = 1-abs(angle_sun/3.1415);

   // vec3 scattering = iv_func(cam_pos, frag_pos)+ground_colour*lum*exp(-t_func_simpler(atm_density, angle_sun_norm));
   vec3 scattering = iv_func(cam_pos, frag_pos)+ground_colour*lum*exp(-texture(tex_t_func, vec2(atm_density, angle_sun_norm) ).rgb );

   color = vec4(scattering, 1.0);
}

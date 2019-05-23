#version 330 core
uniform sampler2D tex_normal_map;
uniform sampler2D tex_depth_buffer;
//uniform sampler2DMS tex;
uniform float tex_width;
uniform float tex_height;
uniform mat4 projection;
uniform mat4 view;
uniform uint AO_effect;

in vec2 uv;
out vec3 color;

float get_sphere_ao(int nb_samples, float ao_sphere_size, bool activate_noise, bool distrib, bool single_dir);
float get_sphere_ao_noisy(int nb_samples);

vec2 co = vec2(1.0); //for the rand func

float rand(){
   float rval = fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
   co += vec2(+1.4252, -1.4324);
   return rval;
}

void main(void){

   const int NB_AO_SAMPLE = 40;

   float depth = texture(tex_depth_buffer, uv).r;

   co += vec2(-depth*100, depth*50); //add random with depth

   float AO = 0.0;

   if(AO_effect == 1u){
      AO = get_sphere_ao(NB_AO_SAMPLE, 0.5, false, false, false);
   }
   else if(AO_effect == 2u){
      AO = get_sphere_ao(NB_AO_SAMPLE, 0.5, true, false, false);
   }
   else if(AO_effect == 3u){
      AO = get_sphere_ao(NB_AO_SAMPLE, 0.5, true, true, false);
      AO = 0.5+AO;
   }
   else if(AO_effect == 4u){
      AO = get_sphere_ao(NB_AO_SAMPLE, 1.0, true, true, true);
   }
   else{
      AO = 1.0;
   }

   //find point position in space
   float z = texture(tex_depth_buffer, uv).r;
   float x = uv.x*2-1;
   float y = uv.y*2-1;
   vec4 screen_coord = vec4(x, y, z, 1.0);
   vec4 unprojected = inverse(projection)*screen_coord;
   vec3 clip_pos = unprojected.xyz/unprojected.w;

   color = vec3(AO);
   vec4 new_proj = (projection*vec4(clip_pos+vec3(3.0, 3.0, 0.0), 1.0));
   vec2 new_screen_pos = new_proj.xy/vec2(new_proj.w);
}

//distrib: concentrate rays to the origin
//signle dir: use hemisphere instead of sphere
float get_sphere_ao(int nb_samples, float ao_sphere_size, bool activate_noise, bool distrib, bool single_dir){
   float AO = 0.0;

   vec3 normal = texture(tex_normal_map, uv).rgb;
   //find point position in space
   float z = texture(tex_depth_buffer, uv).r;
   float x = uv.x*2-1;
   float y = uv.y*2-1;
   vec4 screen_coord = vec4(x, y, z, 1.0);
   vec4 unprojected = inverse(projection)*screen_coord;
   vec3 real_pos = unprojected.xyz/unprojected.w;

   if (z == 1.0){
      return 1.0;
   }

   //do as if we had a 2x2 random texture to "mix" the rotation of the ssao samples
   //normaly it would be a rotation vector (vec3) but here we just call rand() a fixed
   //amount of time which changes the randomness
   if(activate_noise){

      vec2 vals = vec2(uv.x*tex_width, uv.y*tex_height);
      vec2 ivals = floor(vals);
      vec2 mvals = mod(ivals, 2);
      co += ivals;
   }

   vec3 normal_hemisphere = vec3(0.0001, 1.0, 0.001);
   vec3 v = cross(normal_hemisphere, normal);
   float s = length(v);
   float c = dot(normal_hemisphere, normal);
   mat3 v_x = mat3(0, v.z, -v.y, -v.z, 0, v.x, v.y, -v.x, 0);

   mat3 rot_mat = mat3(1.0)+v_x+v_x*v_x*((1-c)/(s*s));

   //gram schmidt algorithm
   vec3 rvec = vec3(0.0, 1.0, 0.0);
   vec3 tangent = normalize(rvec - normal * dot(rvec, normal));
   vec3 bitangent = cross(normal, tangent);
   mat3 tbn = mat3(tangent, bitangent, normal);

   for(int i = 0; i < nb_samples; i++)
   {
      vec3 ao_sample = vec3(0.0);

      if(!single_dir){ //sphere sampling
         ao_sample = vec3(rand()*2-1, rand()*2-1, rand()*2-1); //0;1 to -1;1
         normalize(ao_sample);

      }
      else{ //hemisphere sampling
         ao_sample = vec3(rand()*2-1, rand(), rand()*2-1); //0;1 to -1;1
         normalize(ao_sample);

         if(c > 0.999){
            rot_mat = mat3(1.0);
         }

         ao_sample = rot_mat*ao_sample;
      }

      if(distrib){
         float scale = float(i)/float(nb_samples);
         scale = mix(0.1f, 1.0f, scale * scale);
         ao_sample *= scale;
      }

      ao_sample *= ao_sphere_size; //ao_sample in unit circle

      vec3 sample_pos = vec3(real_pos) + ao_sample;
      vec4 projected_sample = (projection*vec4(sample_pos, 1.0));

      vec2 clip_pos = projected_sample.xy/projected_sample.w;
      vec2 screen_pos = clip_pos*0.5+vec2(0.5);

      float depth_sample = projected_sample.z/projected_sample.w;

      float texture_depth = texture( tex_depth_buffer, screen_pos ).x;

      //find point position in space
      float z = texture_depth;
      float x = uv.x*2-1;
      float y = uv.y*2-1;
      vec4 screen_coord = vec4(x, y, z, 1.0);
      vec4 unprojected = inverse(projection)*screen_coord;
      vec3 texture_real_pos = unprojected.xyz/unprojected.w;

      if ( depth_sample < texture_depth ){ //depth sample nearer to the screen than texture depth
         AO += 1.0;
      }
      else{
         if (distance(sample_pos, texture_real_pos) > ao_sphere_size){
            AO += 1.0;
         }
         AO += 0.0;
      }

   }

   AO = AO/nb_samples;

   return AO;
}

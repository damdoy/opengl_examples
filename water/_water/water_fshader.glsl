#version 330 core

in vec3 frag_position;
in vec2 uv_frag;

uniform vec3 camera_position;
uniform vec3 camera_direction;
uniform vec3 light_position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D shadow_buffer_tex;
uniform sampler2D tex_depth_refraction;
uniform sampler2D tex_refraction;
uniform sampler2D tex_reflection;

uniform float time;
uniform uint water_effect;

in vec3 frag_normal_transformed;
in float red;
in float green;
in float blue;
out vec4 color;

in vec4 shadow_coord;

float get_wave_0(float x, float y){
   x = x+time/100;
   y = y+time/100;
   float wave = sin(x*64*3.1415)*sin(y*16*3.1415);
   wave = wave/2;
   wave = wave*0.01;
   wave += (sin((x+0.23-time/200)*64*3.1415)*sin((y+0.76+time/200)*128*3.1415))/2*0.001;
   wave += (sin((x+0.7664+time/50)*128*3.1415)*sin((y+0.2346-time/50)*128*3.1415))/2*0.0005;
   return wave;
}

//a*sin(d*(x,y)*w+t*phi)
float get_wave_1(float x, float y){
   float wave = 0.01*sin( dot(normalize(vec2(1,0)), vec2(x, y)) *128+time);
   wave += 0.01*sin( dot(normalize(vec2(15,1)), vec2(x, y)) *128+time);
   wave += 0.01*sin( dot(normalize(vec2(10,-1)), vec2(x, y)) *256+time*1.5);
   wave += 0.01*sin( dot(normalize(vec2(3,1)), vec2(x, y)) *256+time*1.5);
   wave += 0.02*sin( dot(normalize(vec2(6,-1)), vec2(x, y)) *64+time*0.8);
   wave += 0.015*sin( dot(normalize(vec2(8,1)), vec2(x, y)) *128+time*0.7);
   return wave/12;
}

float get_wave(float x, float y){
   float wave = 0;
   if(water_effect == 0u){
      wave = get_wave_0(x, y);
   }
   else if(water_effect == 1u){
      wave = get_wave_1(x, y);
   }
   else if(water_effect == 2u){
      wave = 2*get_wave_0(x, y);
   }
   else if(water_effect == 3u){
      wave = 2*get_wave_1(x, y);
   }
   return wave;
}

void main(){

   float wave = get_wave(uv_frag.x, uv_frag.y);

   vec3 pos_before_x = vec3(uv_frag.x-0.01, get_wave(uv_frag.x-0.01, uv_frag.y), uv_frag.y);
   vec3 pos_after_x = vec3(uv_frag.x+0.01, get_wave(uv_frag.x+0.01, uv_frag.y), uv_frag.y);
   vec3 pos_before_y = vec3(uv_frag.x, get_wave(uv_frag.x, uv_frag.y-0.01), uv_frag.y-0.01);
   vec3 pos_after_y = vec3(uv_frag.x, get_wave(uv_frag.x, uv_frag.y+0.01), uv_frag.y+0.01);

   //get normal of wave, for lighting purpose
   vec3 normal_wave = normalize(cross( pos_after_x-pos_before_x, pos_after_y-pos_before_y));

   vec3 light_dir = normalize(light_position-frag_position);
   float diffuse_light = 0.0;

   diffuse_light = dot(normal_wave, light_dir);
   float light_dist = length(light_position-frag_position);
   diffuse_light /= 1+pow(light_dist, -0.5);

   //reflexion of light for specular light calculation, not the image reflexion
   vec3 reflexion = 2*normal_wave*dot(normal_wave, light_dir)-light_dir;
   reflexion = normalize(reflexion);
   vec3 view_dir = normalize(camera_position-frag_position);

   float spec_light = pow(max(dot(reflexion, view_dir), 0.0), 128);
   spec_light = clamp(spec_light, 0.0, 1.0);

   float lum = 0.8*diffuse_light+spec_light;

   vec4 screen_pos = projection*view*vec4(frag_position, 1.0);
   vec2 corr_screen_pos_refraction = screen_pos.xy*0.5/screen_pos.w+vec2(0.5, 0.5);
   vec2 corr_screen_pos_reflection = vec2(corr_screen_pos_refraction.x, 1-corr_screen_pos_refraction.y); //must invert y

   //distort the reflection depending on the wave
   corr_screen_pos_refraction += vec2(wave, wave);
   corr_screen_pos_reflection += vec2(wave, wave);

   vec3 colour_refraction = texture( tex_refraction, corr_screen_pos_refraction ).rgb;
   vec3 colour_reflection = texture( tex_reflection, corr_screen_pos_reflection ).rgb;
   vec3 depth_refraction = texture( tex_depth_refraction, corr_screen_pos_refraction ).rbg;

   float far = 1000;
   float near = 0.1;

   float water_depth_lin = gl_FragCoord.z/gl_FragCoord.w;

   float depth_refraction_lin = 2*near*far/(far+near-(2.0*depth_refraction.x-1.0)*(far-near));

   vec3 direction_camera = normalize(camera_position-frag_position);
   float angle_view = abs(dot(direction_camera, frag_normal_transformed)); //take the camera normal, 1: perpendicular, 0: along the water line
   clamp(angle_view, 0.0, 1.0);
   angle_view = pow(angle_view, 2); //higher pow will means, the area of view where we see the bottom of water is smaller

   float depth_water = depth_refraction_lin-water_depth_lin;
   depth_water = depth_water/8; //shorter distance for the depth (a bit too big for this depth)

   //depth should go from 0 to 1
   vec3 colour_depth = (1.0-depth_water)*colour_refraction+depth_water*vec3(0.5, 0.5, 0.7);

   color.a = 1.0;
   color.rgb = (1-angle_view)*colour_reflection + angle_view*colour_depth + 0.5*diffuse_light+spec_light;

}

void main_colors_const(){
   color = vec4(red, green, blue, 0.0);
}

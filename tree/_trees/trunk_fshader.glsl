#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 light_position;

uniform sampler2D tex;

in float red;
in float green;
in float blue;
out vec4 color;

in vec3 frag_position;
in vec3 frag_normal;
in vec4 shadow_coord;
in vec2 tex_coord;

uniform uint window_width;
uniform uint window_height;
uniform uint shadow_mapping_effect;

uniform vec3 shape_color;

uniform uint shadow_buffer_tex_size;

uniform vec3 sun_dir;
uniform vec3 sun_col;

float get_shadow_val(float offset_x, float offset_y);

vec3 get_billin(vec2 pos, vec3 min, vec3 max);

mat2 noise_ao;

vec3 bark_high = vec3(0.6, 0.4, 0.2);
vec3 bark_low = vec3(0.4, 0.25, 0.2);

void main(){

   float shadow = 1;

   vec3 light_dir = normalize(light_position-frag_position);
   float dist_light = distance(light_position, frag_position);
   float diffuse_light = dot(frag_normal, light_dir);
   float light_intensity = diffuse_light*1/(dist_light/8);

   float diffuse_sun = dot(frag_normal, -sun_dir);
   diffuse_sun = clamp(diffuse_sun, 0.0, 1.0);
   vec3 sun_intensity = sun_col*diffuse_sun;

   vec3 final_intensity = vec3(0.0);

   final_intensity.r = sun_intensity.r;
   final_intensity.g = sun_intensity.g;
   final_intensity.b = sun_intensity.b;

   //use texture or simple generated bilin function
   vec3 texture_col = get_billin(tex_coord*25, bark_low, bark_high);

   float time_of_day = dot(-sun_dir, vec3(0, 1, 0));

   vec3 color_trunk = texture_col*diffuse_light;

   color = vec4( color_trunk*shadow , 1.0);
}

float rand(ivec2 pt)
{
   float x = pt.x*3.13;
   float y = pt.y*7.17;
   return fract(x*y/17);
}

float rand(vec2 pt)
{
   return rand(ivec2(pt.x, pt.y));
}

vec3 get_billin(vec2 pos, vec3 min, vec3 max)
{
   vec2 rounded_pos = round(pos);

   float val00 = rand((pos));
   float val10 = rand((vec2(pos.x, pos.y+1)));
   float val01 = rand((vec2(pos.x+1, pos.y)));
   float val11 = rand((vec2(pos.x+1, pos.y+1)));

   vec2 pos_fract = fract(pos);

   //2d linear interpolation
   float val_x0 = mix(val00, val01, pos_fract.x);
   float val_x1 = mix(val10, val11, pos_fract.x);

   float final_val = mix(val_x0, val_x1, pos_fract.y);

   return mix(max, min, final_val);
}

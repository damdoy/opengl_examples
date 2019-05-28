#version 330 core

in vec3 frag_surface_normal_color;

uniform sampler2D shadow_buffer_tex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 light_position;

in float red;
in float green;
in float blue;
out vec4 color;

in vec3 frag_position;
in vec3 frag_normal;
in vec4 shadow_coord;

uniform uint window_width;
uniform uint window_height;
uniform uint shadow_mapping_effect;

uniform vec3 shape_color;

uniform uint shadow_buffer_tex_size;

uniform vec3 sun_dir;
uniform vec3 sun_col;

void main(){

   vec3 light_dir = normalize(light_position-frag_position);
   float dist_light = distance(light_position, frag_position);
   float diffuse_light = dot(frag_normal, light_dir);
   float light_intensity = diffuse_light*1/(dist_light/8);

   float diffuse_sun = dot(frag_normal, -sun_dir);
   diffuse_sun = clamp(diffuse_sun, 0.0, 1.0);
   vec3 sun_intensity = sun_col*diffuse_sun;

   vec3 final_intensity = vec3(0.0);
   final_intensity.r = max(light_intensity, sun_intensity.r);
   final_intensity.g = max(light_intensity, sun_intensity.g);
   final_intensity.b = max(light_intensity, sun_intensity.b);

   color = vec4(final_intensity*shape_color, 1.0);
}

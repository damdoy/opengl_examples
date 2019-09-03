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

void main(){

   vec3 light_dir = normalize(light_position-frag_position);
   float diffuse_light = dot(frag_normal, light_dir);

   color = vec4(diffuse_light*shape_color, 1.0);
}

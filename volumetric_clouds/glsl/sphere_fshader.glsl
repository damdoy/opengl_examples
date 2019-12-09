#version 330 core

in vec3 frag_surface_normal_color;

in vec3 frag_normal_transformed;
in float frag_spot_range;
in float frag_spot_pow;

uniform vec3 camera_position;
uniform vec3 light_position;
uniform vec3 sun_dir;
uniform bool activate_spot;

out vec4 color;

void main(){
   vec3 light_dir = sun_dir;
   float diffuse_light = 0.0;

   diffuse_light = dot(frag_normal_transformed, light_dir);

   float lum = 0.8*diffuse_light;

   color = vec4(lum, lum, lum, 1.0);

}

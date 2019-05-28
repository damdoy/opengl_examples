#version 330 core

in vec3 position;
in vec3 surface_normal;
in vec3 vertex_normal;
uniform vec3 light_position;
uniform vec3 camera_position;
uniform vec4 clip_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 frag_position;
out vec3 frag_normal_transformed;

out vec4 shadow_coord;
out vec3 sun_dir_corr;

uniform vec3 sun_dir;

void main(){
   gl_Position = projection*view*model*vec4(position, 1.0);
   gl_ClipDistance[0] = dot(model*vec4(position, 1.0), clip_coord);

   frag_position = vec3(model*vec4(position, 1.0));

   sun_dir_corr = sun_dir;
}

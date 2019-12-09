#version 330 core

in vec3 position;
in vec2 uv;
in vec3 pos_point;
in vec2 life_age;
in float particle_colour;
in float particle_random;
in uint particle_active;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_position;
uniform vec4 clip_coord;

out vec2 frag_uv;
out float frag_colour;
out float random;
flat out uint frag_active;

void main(){


   frag_active = particle_active;

   //from http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/
   //to have billboards sprites
   vec3 CameraRight_worldspace = vec3(view[0][0], view[1][0], view[2][0]);
   vec3 CameraUp_worldspace = vec3(view[0][1], view[1][1], view[2][1]);

   float size = 24;

   vec3 transf_pos_point = vec3(model*vec4(pos_point, 1.0));

   vec3 vertexPosition_worldspace =
       transf_pos_point
       + CameraRight_worldspace * position.x * size
       + CameraUp_worldspace * position.z * size;

   gl_Position = projection*view*vec4(vertexPosition_worldspace, 1.0);
   random = particle_random;
   frag_uv = uv;
   frag_colour = particle_colour;
}

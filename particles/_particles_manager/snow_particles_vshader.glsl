#version 330 core

in vec3 position;
in vec2 uv;
in vec3 pos_point;
in vec2 life_age;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_position;
uniform vec4 clip_coord;

out vec2 frag_uv;
out float frag_life;
out float frag_age;

void main(){

   //from http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/
   //to have billboards sprites
   vec3 CameraRight_worldspace = vec3(view[0][0], view[1][0], view[2][0]);
   vec3 CameraUp_worldspace = vec3(view[0][1], view[1][1], view[2][1]);

   float size = 0.25;

   vec3 vertexPosition_worldspace =
       pos_point
       + CameraRight_worldspace * position.x * size
       + CameraUp_worldspace * position.z * size;

   gl_Position = projection*view*vec4(vertexPosition_worldspace, 1.0);
   frag_uv = uv;
   frag_life = life_age[0];
   frag_age = life_age[1];
}

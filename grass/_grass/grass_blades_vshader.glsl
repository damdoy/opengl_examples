#version 330 core

in vec3 position;
in vec2 uv;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_position;

uniform sampler2D tex_wind; //wind speed texture
uniform float min_pos;
uniform float max_pos;

uniform vec2 wind_dir;
in mat4 model_mat;

out vec3 frag_position;
out vec3 frag_normal_transformed;
out float frag_diffuse_light;

out vec4 shadow_coord;
out vec2 frag_uv;
out vec2 global_uv;

void main(){

   mat4 model = model_mat;
   vec3 new_pos = vec3(model*vec4(position, 1.0));

   float span_texture = max_pos-min_pos;
   vec2 relative_tex_pos = vec2(new_pos.x/100+0.5, new_pos.z/100+0.5);
   global_uv = relative_tex_pos;
   relative_tex_pos /= 4;

   float wind_x = texture(tex_wind, relative_tex_pos+wind_dir ).r;
   float wind_z = texture(tex_wind, relative_tex_pos+vec2(0.5, 0.5)+wind_dir ).r;

   //apply the wind, on the higher vertices, stronger wind
   if(gl_VertexID == 0){
      wind_x *= 3.0;
      wind_z *= 3.0;
      new_pos.x += wind_x;
      new_pos.y -= (abs(wind_x)+abs(wind_z))*0.5;
      new_pos.z += wind_z;
   }
   if(gl_VertexID == 1 || gl_VertexID == 2 ){
      wind_x *= 1.5;
      wind_z *= 1.5;
      new_pos.x += wind_x;
      new_pos.y -= (abs(wind_x)+abs(wind_z))*0.5;
      new_pos.z += wind_z;
   }
   if(gl_VertexID == 3 || gl_VertexID == 4 ){
      wind_x *= 0.5;
      wind_z *= 0.5;
      new_pos.x += wind_x;
      new_pos.y -= (abs(wind_x)+abs(wind_z))*0.5;
      new_pos.z += wind_z;
   }

   gl_Position = projection*view*vec4(new_pos, 1.0);

   vec3 model_pos = vec3(model*vec4(position, 1.0));
   frag_position = model_pos;

   frag_uv = uv;
}

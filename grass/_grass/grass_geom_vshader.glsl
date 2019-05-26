#version 330 core

uniform mat4 view;
uniform mat4 projection;

in mat4 model_mat;

out vec2 geom_global_uv;
out mat4 geom_model_matrix;
out mat4 geom_view_matrix;
out mat4 geom_projection_matrix;

void main(){
   //passthrough shader, vertex calculations will be done in the geometry shader

   vec3 new_pos = vec3(model_mat*vec4(vec3(0, 0, 0), 1.0));
   vec2 relative_tex_pos = vec2(new_pos.x/100+0.5, new_pos.z/100+0.5);
   geom_global_uv = relative_tex_pos;
   geom_model_matrix = model_mat;
   geom_view_matrix = view;
   geom_projection_matrix = projection;
}

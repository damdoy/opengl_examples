#version 330 core

in vec3 position;
in vec3 surface_normal;
in vec3 vertex_normal;
uniform vec3 light_position;
uniform vec3 camera_position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 frag_normal_transformed;
out vec3 frag_pos;

void main(){
   gl_Position = projection*view*model*vec4(position, 1.0);
   frag_pos = vec3(model*vec4(position, 1.0));

   //model matrix for normals needs to be ((mat)⁻¹)^T
   mat3 normalMat = mat3(model);
   normalMat = transpose(inverse(normalMat));

   vec3 normal_transformed = normalize(normalMat*vertex_normal);
   normal_transformed = clamp(normal_transformed, 0.0, 1.0);

   frag_normal_transformed = normal_transformed;

}

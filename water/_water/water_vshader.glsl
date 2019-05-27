#version 330 core

in vec3 position;
in vec2 uv;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_position;

uniform mat4 shadow_matrix; //bias*P*V

in vec3 surface_normal;

out vec3 frag_position;
out vec3 frag_normal_transformed;
out vec2 uv_frag;
out float red;
out float green;
out float blue;

out vec4 shadow_coord;

void main(){
   gl_Position = projection*view*model*vec4(position, 1.0);

   mat3 normalMat = mat3(model);
   normalMat = transpose(inverse(normalMat));

   vec3 normal_transformed = vec3(0.0);
   normal_transformed = normalize(normalMat*surface_normal);

   frag_normal_transformed = normal_transformed;
   frag_position = vec3(model*vec4(position, 1.0));

   shadow_coord = shadow_matrix * model*vec4(position, 1.0);

   uv_frag = uv;
}

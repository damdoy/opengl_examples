#version 330 core

in vec3 position;
in vec3 vertex_normal;
uniform vec3 light_position;
uniform vec3 camera_position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 frag_position;
out vec3 frag_surface_normal_color;
out vec3 frag_normal_transformed;

//to draw the height, without having it modified by the model matrix
out float frag_nontransfheight;

void main(){
   gl_Position = projection*view*model*vec4(position, 1.0);

   //matrix to adapts the normals to the application of matrix on the terrain
   //otherwise the terrain is bad
   mat3 normalMat = mat3(model);
   normalMat = transpose(inverse(normalMat));

   vec3 normal_transformed = vec3(0.0);
   normal_transformed = normalize(normalMat*vertex_normal);


   ////////////////////
   //per vertex lighting
   /////////////////
   // vec3 light_dir = normalize(light_position-vec3(model*vec4(position, 1.0)));
   // vec3 view_dir = normalize(camera_position-vec3(model*vec4(position, 1.0)));
   //
   // float diffuse_light = 0.0;
   // float spec_light = 0.0;
   // diffuse_light = dot(normal_transformed, light_dir);
   //
   // vec3 reflexion = 2*normal_transformed*dot(normal_transformed, light_dir)-light_dir;
   // reflexion = clamp(reflexion, 0.0, 1.0);
   // spec_light = pow(dot(reflexion, view_dir), 64);
   // spec_light = clamp(spec_light, 0.0, 1.0);
   //
   // float lum = 0.8*diffuse_light+0.8*spec_light;
   // lum = clamp(lum, 0.0, 1.0);

   frag_normal_transformed = normal_transformed;
   frag_position = vec3(model*vec4(position, 1.0));

   frag_nontransfheight = position[1];
}

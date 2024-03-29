#version 330 core

in vec3 position;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_position;
in vec3 surface_normal;

uniform mat4 shadow_matrix; //bias*P*V

out vec3 frag_surface_normal_color;
out float red;
out float green;
out float blue;

out vec4 shadow_coord;

void main(){
   gl_Position = projection*view*model*vec4(position, 1.0);
   // gl_Position = projection*view*vec4(0,0,0, 1.0);

   mat3 normalMat = mat3(model);
   normalMat = transpose(inverse(normalMat));

   vec3 light_dir = normalize(light_position-vec3(model*vec4(position, 1.0)));

   vec3 normal_transformed = vec3(0.0);
   normal_transformed = normalize(normalMat*surface_normal);

   float diffuse_light = 0.0;
   diffuse_light = dot(normal_transformed, light_dir);

   float lum = 1.0*diffuse_light;
   lum = clamp(lum, 0.0, 1.0);
   frag_surface_normal_color = vec3(lum, lum, lum);

   shadow_coord = shadow_matrix * model*vec4(position, 1.0);
}

void main_colors_const(){
   gl_Position = projection*view*model*vec4(position, 1.0);
   red = green = blue = 0.0;

   if(gl_VertexID == 0){
      red = 1.0;
   }
   else if(gl_VertexID == 1){
      green = 1.0;
   }
   else if(gl_VertexID == 2){
      blue = 1.0;
   }
}

#version 330 core

in vec3 position;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out float red;
out float green;
out float blue;

void main(){
   gl_Position = projection*view*model*vec4(position, 1.0);
   // gl_Position = vec4(position, 1.0);
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

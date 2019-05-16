#version 330 core 

in vec3 vpoint;
out float red;
out float green;
out float blue;

void main(){
   gl_Position = vec4(vpoint, 1.0);
   red = green = blue = 0.0;

   if(gl_VertexID == 0){
      red = 1.0;
   }
   else if(gl_VertexID == 1){
      green = 1.0;
   }
   else{
      blue = 1.0;
   }
}

#version 330 core

in vec3 frag_normal_transformed;
in vec3 frag_position;

uniform vec3 camera_position;
uniform vec3 light_position;

uniform bool activate_colour;
uniform bool activate_heightmap;
uniform bool activate_wireframe;

out vec3 color;

in float frag_nontransfheight;

const vec3 COLOR[3] = vec3[](
    vec3(0.0, 0.3, 1.0),
    vec3(0.0, 0.7, 0.0),
    vec3(0.8, 0.8, 0.8));

const float ambiant_light = 0.3;

void main(){
   vec3 light_dir = normalize(light_position-frag_position);
   float diffuse_light = 0.0;
   // float spec_light = 0.0;

   vec3 reflexion = 2*frag_normal_transformed*dot(frag_normal_transformed, light_dir)-light_dir;
   reflexion = normalize(reflexion);
   vec3 view_dir = normalize(camera_position-frag_position);

   // spec_light = pow(max(dot(reflexion, view_dir), 0.0), 64);
   // spec_light = clamp(spec_light, 0.0, 1.0);

   diffuse_light = dot(frag_normal_transformed, light_dir);
   diffuse_light = clamp(diffuse_light, 0.0, 1.0);

   float lum = 0.8*diffuse_light+/*spec_light+*/ambiant_light;
   clamp(lum, 0.0, 1.0);

   float height = frag_nontransfheight/2.0+0.5;
   vec3 pixel_colour = vec3(0.0);

   if(height < 0.5){
      pixel_colour = (1.0-height*2)*COLOR[0]+height*2*COLOR[1];
   }else{
      pixel_colour = (1.0-(height-0.5)*2)*COLOR[1]+(height-0.5)*2*COLOR[2];
   }

   if(!activate_colour){
      pixel_colour = vec3(1.0);
   }

   else{
      color = pixel_colour*lum;
   }

   //we want black lines if wireframe is activated
   if (activate_wireframe){
      color = vec3(0.0);
   }
}

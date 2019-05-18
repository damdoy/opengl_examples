#version 330 core

in vec3 vpoint;
in vec3 normal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 light_position;
uniform vec3 camera_position;
uniform uint lighting_mode;
uniform bool activate_specular;
uniform vec3 spot_direction;
uniform bool activate_spot;

out float red;
out float green;
out float blue;

out vec3 frag_position;
out vec3 frag_surface_normal_color;
out vec3 frag_normal_transformed;
out float frag_spot_range;
out float frag_spot_pow;

const float spot_range = 0.98; //how wide the spot should be (in term of dot(light_dir, -spot_dir))
const float spot_pow = 128;

void main(){
   gl_Position = projection*view*model*vec4(vpoint, 1.0);
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

   gl_Position = projection*view*model*vec4(vpoint, 1.0);

   //model matrix for normals needs to be ((mat)⁻¹)^T
   mat3 normalMat = mat3(model);
   normalMat = transpose(inverse(normalMat));

   //find light and view dir from the vertex
   vec3 light_dir = normalize(light_position-vec3(model*vec4(vpoint, 1.0)));
   vec3 view_dir = normalize(camera_position-vec3(model*vec4(vpoint, 1.0)));

   vec3 normal_transformed = vec3(0.0);
   float diffuse_light = 0.0;

   //per vertex or per pixel
   normal_transformed = normalize(normalMat*normal);
   normal_transformed = clamp(normal_transformed, 0.0, 1.0);

   //calculate per vertex diffuse light
   diffuse_light = dot(normal_transformed, light_dir);

   float spot_brightness = 1.0;

   if(activate_spot == true){
      vec3 spot_direction_norm = normalize(spot_direction);
      float cos_spot = dot(-spot_direction_norm, light_dir);

      if(cos_spot > spot_range){
         spot_brightness = 1.0;
      }
      else{
         spot_brightness = pow(cos_spot+(1.0-spot_range), spot_pow);
      }
   }

   //calculate a sum for the lighting value of either per vertex or surface
   float lum = 0.8*diffuse_light;
   lum = lum*spot_brightness;
   lum = clamp(lum, 0.0, 1.0);

   //calculations of transforms, positions for the per pixel lighting
   frag_normal_transformed = normal_transformed;
   frag_position = vec3(model*vec4(vpoint, 1.0));
   frag_surface_normal_color = vec3(lum, lum, lum);
   frag_spot_range = spot_range;
   frag_spot_pow = spot_pow;

}

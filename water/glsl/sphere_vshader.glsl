#version 330 core

in vec3 position;
in vec3 surface_normal;
in vec3 vertex_normal;
uniform vec3 light_position;
uniform vec3 camera_position;
uniform vec4 clip_coord;
uniform uint lighting_mode;
uniform bool activate_specular;
uniform vec3 spot_direction;
uniform bool activate_spot;

uniform mat4 shadow_matrix; //bias*P*V

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 frag_position;
out vec3 frag_surface_normal_color;
out vec3 frag_normal_transformed;
out float frag_spot_range;
out float frag_spot_pow;

out vec4 shadow_coord;

const float spot_range = 0.98; //hhow wide the spot should be (in term of dot(light_dir, -spot_dir))
const float spot_pow = 128;

void main(){
   gl_Position = projection*view*model*vec4(position, 1.0);
   gl_ClipDistance[0] = dot(model*vec4(position, 1.0), clip_coord);

   mat3 normalMat = mat3(model);
   normalMat = transpose(inverse(normalMat));

   vec3 light_dir = normalize(light_position-vec3(model*vec4(position, 1.0)));
   vec3 view_dir = normalize(camera_position-vec3(model*vec4(position, 1.0)));

   vec3 normal_transformed = vec3(0.0);
   float diffuse_light = 0.0;
   float spec_light = 0.0;

   if(lighting_mode == 0u){ // per surface
      normal_transformed = normalize(normalMat*surface_normal);
   }
   else if(lighting_mode == 1u){
      normal_transformed = normalize(normalMat*vertex_normal);
   }
   else{
      normal_transformed = normalize(normalMat*vertex_normal);
   }

   //normal_transformed = clamp(normal_transformed, 0.0, 1.0);

   diffuse_light = dot(normal_transformed, light_dir);

   if(activate_specular == true){
      vec3 reflexion = 2*normal_transformed*dot(normal_transformed, light_dir)-light_dir;
      reflexion = clamp(reflexion, 0.0, 1.0);
      spec_light = pow(dot(reflexion, view_dir), 64);
      spec_light = clamp(spec_light, 0.0, 1.0);
   }

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

   float lum = 0.8*diffuse_light+0.8*spec_light;
   lum = lum*spot_brightness;
   lum = clamp(lum, 0.0, 1.0);

   shadow_coord = shadow_matrix * model*vec4(position, 1.0);

   frag_normal_transformed = normal_transformed;
   frag_position = vec3(model*vec4(position, 1.0));
   frag_surface_normal_color = vec3(lum, lum, lum);
   frag_spot_range = spot_range;
   frag_spot_pow = spot_pow;

}

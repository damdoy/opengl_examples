#version 330 core

in vec3 frag_position;

uniform vec3 camera_position;
uniform vec3 light_position;

in vec2 uv;
uniform sampler2D tex;

out vec3 color;

void main(){
   vec3 light_dir = normalize(light_position-frag_position);
   float light_length = length(light_position-frag_position);
   float diffuse_light = 0.0;
   float spec_light = 0.0;

   vec3 frag_normal_transformed = vec3(0.0, 1.0, 0.0);

   //find normal from texture
   const float EPSILON_DERIVATIVE = 1.0/1024.0;
   float point_x_before = texture(tex, uv+vec2(-EPSILON_DERIVATIVE, 0.0)).r;
   float point_x_after = texture(tex, uv+vec2(EPSILON_DERIVATIVE, 0.0)).r;
   float point_y_before = texture(tex, uv+vec2(0.0, -EPSILON_DERIVATIVE)).r;
   float point_y_after = texture(tex, uv+vec2(0.0, EPSILON_DERIVATIVE)).r;

   vec3 der_x = normalize(vec3(2*EPSILON_DERIVATIVE, point_x_after-point_x_before, 0.0));
   vec3 der_y = normalize(vec3(0.0, point_y_after-point_y_before, 2*EPSILON_DERIVATIVE));

   vec3 normal = normalize(cross(der_y, der_x));

   //by adding to the y component, we make the surface less "rough"
   frag_normal_transformed = normalize(normal+vec3(0.0, 1.5, 0.0));

   //calculations for specular light
   vec3 reflexion = 2*frag_normal_transformed*dot(frag_normal_transformed, light_dir)-light_dir;
   reflexion = normalize(reflexion);
   vec3 view_dir = normalize(camera_position-frag_position);

   spec_light = pow(max(dot(reflexion, view_dir), 0.0), 16);
   spec_light = clamp(spec_light, 0.0, 1.0);

   diffuse_light = dot(frag_normal_transformed, light_dir)/(light_length/2.0);
   diffuse_light = clamp(diffuse_light, 0.0, 1.0);

   float lum = 1.0*diffuse_light+0.5*spec_light;
   lum = clamp(lum, 0.0, 1.0);

   color = 1.0*vec3(lum)+0.0*vec3(texture(tex, uv).r);
}

#version 330 core

in vec3 position;
in vec3 surface_normal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 normal;
out vec3 view_normal;

void main(){
   gl_Position = projection*view*model*vec4(position, 1.0);
   normal = vec3( transpose(inverse(model))*vec4(surface_normal, 1.0) );
   normal = normalize(normal);
   view_normal = vec3( transpose(inverse(view))*vec4(normal, 1.0) );
   view_normal = normalize(view_normal);
}

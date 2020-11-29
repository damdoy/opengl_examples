#version 400 core

out vec3 color;

in vec2 uv;
in vec3 normal;
uniform sampler2D tex;

void main(){
   vec3 sun_dir = normalize(vec3(1,1,1));
   color = vec3( dot(normal, sun_dir))/2;

   //possibility of having a texture
   // color = texture(tex, uv).rgb;
}

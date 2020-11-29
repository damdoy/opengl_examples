#version 400 core

uniform mat4 view;
uniform mat4 projection;

//selection of the tessellation
// layout(triangles, fractional_even_spacing, ccw) in;
layout(triangles, fractional_odd_spacing, ccw) in;
// layout(triangles, equal_spacing, ccw) in;

in vec2 uv_tes[];
out vec2 uv;

out vec3 normal;

precise gl_Position;

//do interpolation between the points according to the barycentric values
vec2 mix_tess_vec2(vec2 p0, vec2 p1, vec2 p2){
   return gl_TessCoord.x*p0 + gl_TessCoord.y*p1 + gl_TessCoord.z*p2;
}

vec4 mix_tess_vec4(vec4 p0, vec4 p1, vec4 p2){
   return gl_TessCoord.x*p0 + gl_TessCoord.y*p1 + gl_TessCoord.z*p2;
}

void main(){
   uv = mix_tess_vec2(uv_tes[0], uv_tes[1], uv_tes[2]);

   float sine_frequency = 2;

   vec4 vertex_position = mix_tess_vec4(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position);

   //will do a simple 2D sine terrain
   vertex_position.y += (sin(vertex_position.x*sine_frequency)+sin(vertex_position.z*sine_frequency))*0.3;

   //vectors to calculate the normal of the sine terrain (partial derivative of sin(x))
   vec3 norm_vec_x = vec3(1, cos(sine_frequency*vertex_position.x), 0);
   vec3 norm_vec_z = vec3(0, cos(sine_frequency*vertex_position.z), 1);

   normal = normalize(cross(norm_vec_z, norm_vec_x));

   gl_Position = projection*view*vertex_position;
}

#version 330 core


layout(points) in;

layout(triangle_strip, max_vertices = 7) out;

in vec2 geom_global_uv[];
in mat4 geom_model_matrix[];
in mat4 geom_view_matrix[];
in mat4 geom_projection_matrix[];

out vec2 frag_uv;
out vec2 global_uv;

uniform vec2 wind_dir;
uniform sampler2D tex_wind; //wind speed texture

uint nb_vertice = 7u;

vec3 points[] = vec3[7u](vec3(0.0f, 0.0f, -2.0f), vec3(-1.0f, 0.0f, 0.0f), // 1
vec3(1.0f, 0.0f, 0.0f), // 2
vec3(-1.0f,  0.0f, 1.0f), // 3
vec3(1.0f, 0.0, 1.0f), // 4
vec3(-1.0f, 0.0, 2.0f), // 5
vec3(1.0f, 0.0, 2.0f));

vec2 vtexcoord[] = vec2[7u]( vec2(0.5f, 0.0f), // 0
                              vec2(0.0f, 0.5f), // 1
                              vec2(1.0f, 0.5f), // 2
                              vec2(0.0f, 0.75f), // 3
                              vec2(1.0f, 0.75f), // 4
                              vec2(0.0f, 1.0f), // 5
                              vec2(1.0f, 1.0f)); // 6


uint vpoint_index[7] = uint[7](0u, 1u, 2u, 3u, 4u, 5u, 6u);

vec3 calculate_vertex_wind(vec3 vertex, uint id){

   vec2 relative_tex_pos = vec2(vertex.x/100+0.5, vertex.z/100+0.5);
   relative_tex_pos /= 4;

   float wind_x = texture(tex_wind, relative_tex_pos+wind_dir ).r;
   float wind_z = texture(tex_wind, relative_tex_pos+vec2(0.5, 0.5)+wind_dir ).r;

   //apply the wind, on the higher vertices, stronger wind
   if(id == 0u){
      wind_x *= 3.0;
      wind_z *= 3.0;
      vertex.x += wind_x;
      vertex.y -= (abs(wind_x)+abs(wind_z))*0.5;
      vertex.z += wind_z;
   }
   if(id == 1u || id == 2u ){
      wind_x *= 1.5;
      wind_z *= 1.5;
      vertex.x += wind_x;
      vertex.y -= (abs(wind_x)+abs(wind_z))*0.5;
      vertex.z += wind_z;
   }
   if(id == 3u || id == 4u ){
      wind_x *= 0.5;
      wind_z *= 0.5;
      vertex.x += wind_x;
      vertex.y -= (abs(wind_x)+abs(wind_z))*0.5;
      vertex.z += wind_z;
   }

   return vertex;
}

void main()
{

   for(uint i = 0u; i < nb_vertice; i++)
   {
      frag_uv = vtexcoord[vpoint_index[i]];
      global_uv = geom_global_uv[0];
      //apply model matrix on the point
      vec4 new_pos = geom_model_matrix[0]*(gl_in[0].gl_Position + vec4(points[vpoint_index[i]], 1.0) );

      //apply view and projection matrix
      gl_Position = geom_projection_matrix[0]*geom_view_matrix[0]*vec4(calculate_vertex_wind(vec3(new_pos), i), 1.0);
      EmitVertex();
   }

   EndPrimitive();
}

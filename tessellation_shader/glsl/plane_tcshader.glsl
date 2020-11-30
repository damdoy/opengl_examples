#version 400 core

layout (vertices = 3) out;

uniform vec3 camera_position;

uniform int fixed_tessellation_level_active;
uniform int fixed_tessellation_level;

in vec2 uv_tess[];

out vec2 uv_tes[];

void main()
{
   uv_tes[gl_InvocationID] = uv_tess[gl_InvocationID];

   float dist0 = distance(camera_position, (gl_in[0].gl_Position.xyz+gl_in[1].gl_Position.xyz)*0.5 );
   float dist1 = distance(camera_position, (gl_in[1].gl_Position.xyz+gl_in[2].gl_Position.xyz)*0.5);
   float dist2 = distance(camera_position, (gl_in[2].gl_Position.xyz+gl_in[0].gl_Position.xyz)*0.5);

   gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

   // Calculate the tessellation levels, from distance
   float tess_lvl0 = 150/dist0;
   float tess_lvl1 = 150/dist1;
   float tess_lvl2 = 150/dist2;

   if(fixed_tessellation_level_active != 0){
      tess_lvl0 = fixed_tessellation_level;
      tess_lvl1 = fixed_tessellation_level;
      tess_lvl2 = fixed_tessellation_level;
   }

   //select correct levels for outer level, otherwise some cracks will appear
   gl_TessLevelOuter[0] = tess_lvl1;
   gl_TessLevelOuter[1] = tess_lvl2;
   gl_TessLevelOuter[2] = tess_lvl0;
   gl_TessLevelInner[0] = (tess_lvl0+tess_lvl1+tess_lvl2)/3;
}

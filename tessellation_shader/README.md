# Tessellation shader

Requires OpenGL 4.0

This example shows a bunch of triangles arranged in a flat plane. The height of the verice is calculated from sines, sin(x)+sin(z).

The tessellation level is calculated as the inverse of the distance between the camera and the verices. Moving the camera around will change the tessellation level the closer it is to a geometry, the more detailed it will be.

Most of the calculations that would be done by the vertex shader are done by the tessellation evaluation shader (TES).
Since it will contain all the points, the vertex shader is only applied on the control points, that is the vertices before the tessellation, only the model matrix is applied on the vertices here.

# Controls
- WASD: moves the camera
- IJKL: change camera view direction
- N/M: Wireframe ON/OFF

# Grass

![screenshot1](../screenshots/grass_1.png)

The classical approach to display grass is to use the billboard approach, where two planes of partially transparent textures are
displayed. This has the advantage to have a detailled image of grass (depending on the texture). But is pretty rigid.

In this examples, individual blades of grass are displayed, the advantage to display grass blade by blade is that they can move
more freely. Here a perlin noise is used to simulate wind.

Two approaches are used to display a huge quantity of grass, either instanced drawing or generate the grass in the geometry shader.
In both cases, the model matrices are generated in cpu and stored in a huge buffer. In the instanced drawing case, the same blade is drawn
multiple time with a single draw call. For each drawing instance, a different model matrix from the buffer will be used to position the blade.

In the geometry shader case, a single call is also used, but the geometry is created inside the geometry shader.

The goal of implementing both case was to compare the performance, but they have pretty similar performance on my computer.

The advantage of both these methods is that there are very few cpu-gpu transfer, except the very large matrix buffer done at the initialisation.
This means that a huge quantity of grass can be drawn.
On my computer (Intel HD Graphics 620, with core i7-7500U), I get 60fps for 90k blades of grass and ~18fps for 360k blades of grass for both implementations.

Each blade of grass uses its position in the world to get the wind value, from a perlin noise texture, the wind force is applied on each vertices,
with stronger force on the higher vertices than the lower ones.

# Controls
- WASD: moves the camera
- IJKL: change camera view direction

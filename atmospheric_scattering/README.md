# Atmospheric scattering

![screenshot](../screenshots/atmosphere_1.png) ![screenshot](../screenshots/atmosphere_2.png)

Implementation (with some liberties) of https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter16.html

The optimisations explained in 16.3 were implemented allowing for a responsive framerate (>30) on a very modest GPU (Intel HD Graphics 620).

A simple noise function was used to make some "islands" on the planet.

# Controls
- WASD: moves the camera
- IJKL: change camera view direction
- MN: freezes the sun direction

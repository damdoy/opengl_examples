# OpenGL examples
This is a list of examples done a few years ago after a computer graphic course to better understand opengl and apply some of the effects existing.

Content of the repo:

- Triangle: hello world program, tests the display and the buffers
- Rotating cube: Add movement in the scene and transormation matrices
- Texture plane: Tests textures, both from an image and generated
- Sphere light: Different lightings on a sphere: Flat Goureaud and Phong with interacitvity to select the lighting type
- Terrain camera: A terrain generated with a Perlin noise in which we can navigate with either a free moving camera or a FPS-style one
- Bumpmapping: Simulates bumps on a flat surface using random Perlin noise and pixel shader
- Framebuffer: Displays the scene in a framebuffer, allowing post processing effects: gaussian blur, edge detection, etc.

## How to build
These examples are compiled on a Linux Mint 18.1

Libraries needed are : glfw2, glew 1.13 devIL 1.7.8

Create a `build` folder in the example you want to build, then from the `build` folder, `cmake ..` to populate it and `make` to build the example

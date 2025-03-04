# Hi, I'm a readme file

![image](https://github.com/user-attachments/assets/02378292-1344-40cd-a975-9492c1e2fa94)
(seed: 42)

### Build Process
Install cmake and run ```bash build_debian.sh``` in your terminal. 
This will create a build directory and compile the project.

### Controls
- `WASD` / `SHIFT` / `SPACE` to move around
- `TAB` to toggle debug mode
- `ESC` to quit

### Features
- procedural world generation (perlin noise)
- face culling (backface culling, hollow chunks)
- swept AABB collisions (well.. pls don't test)
- upcoming features
  - placing and breaking blocks
  - frustum culling

### Libraries
- [spdlog](https://github.com/gabime/spdlog) - Logging 
- [glad](https://github.com/Dav1dde/glad) - OpenGL loading
- [glfw](https://github.com/glfw/glfw) - Window managing
- [glm](https://github.com/g-truc/glm) - Quick math library, because I'm not a psychopath
- [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) - Loading texture atlas

### Learning Resources / Credits
- [Ray Casting](http://www.cse.yorku.ca/~amana/research/grid.pdf)
- [OpenGL](https://youtube.com/playlist?list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2&si=crdBuV5tGpnvQw_7)
- [Swept AABB collisions](https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/swept-aabb-collision-detection-and-response-r3084/)
- [Perlin noise](https://www.youtube.com/watch?v=kCIaHqb60Cw)

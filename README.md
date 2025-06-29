# Hi, I'm a readme file

### Build Process
Install cmake and run ```bash build.sh``` in your terminal. 
This will create a build directory and compile the project.

### Controls
- `WASD` / `SHIFT` / `SPACE` to move around
- `TAB` to toggle debug mode
- `ESC` to quit
- `V` disable mouse cursor
- `b` enable mouse cursor
- `x` refresh (drop all chunks)

### Roadmap
- [ ] procedural world generation
  - [x] noise maps for terrain height (for each biome)
  - [ ] noise maps for biome distribution
  - [ ] ...
- [x] pack vertex into 1 int
- [ ] send 1 vertex to draw whole face
- [x] face culling 
- [ ] frustum culling
- [ ] placing and breaking blocks
  - [ ] face selection
  - [ ] block breaking
  - [ ] block placing
- [ ] collisions

### Libraries
- [spdlog](https://github.com/gabime/spdlog) - Logging 
- [glad](https://github.com/Dav1dde/glad) - OpenGL loading
- [glfw](https://github.com/glfw/glfw) - Window managing
- [glm](https://github.com/g-truc/glm) - Quick math library, because I'm not a psychopath
- [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) - Loading texture atlas
- [glText](https://github.com/vallentin/glText) - Text rendering
- [FastNoiseLite](https://github.com/Auburn/FastNoiseLite) - Noise generation

### Learning Resources / Credits
- [Ray Casting](http://www.cse.yorku.ca/~amana/research/grid.pdf)
- [OpenGL](https://youtube.com/playlist?list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2&si=crdBuV5tGpnvQw_7)
- [Swept AABB collisions](https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/swept-aabb-collision-detection-and-response-r3084/)
- [Block selection](https://gamedev.stackexchange.com/questions/47362/cast-ray-to-select-block-in-voxel-game)

# Hi, I'm a readme file

![image](https://github.com/user-attachments/assets/c086933c-c18a-4289-9ee4-a8d35243db03)

### Build Process
Install cmake and run ```bash build.sh``` in your terminal. 
This will create a build directory and compile the project.

### Controls
- `WASD` / `SHIFT` / `SPACE` to move around
- `TAB` to toggle debug mode
- `ESC` to quit
- `V` toggle mouse cursor
- `X` refresh (drop all chunks)

### Roadmap
- [ ] graphics & optimizations
  - [x] 1 int -> 1 vertex
  - [x] 1 int -> whole face
  - [x] imgui debug menu
  - [x] face culling
  - [ ] frustum culling
    - [x] simple circular frustum culling 
    - [ ] more complex version
  - [ ] particle system


- [ ] game mechanics
  - [ ] collisions
  - [ ] infinite world
    - [x] chunk system
    - [x] chunk loading
    - [ ] chunk unloading
  - [x] placing and breaking blocks
  - [ ] procedural world generation
    - [x] noise maps for terrain height (for each biome)
    - [ ] noise maps for biome distribution

### Libraries
- [spdlog](https://github.com/gabime/spdlog) - Logging 
- [glad](https://github.com/Dav1dde/glad) - OpenGL loading
- [glfw](https://github.com/glfw/glfw) - Window managing
- [glm](https://github.com/g-truc/glm) - Quick math library, because I'm not a psychopath
- [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) - Loading texture atlas
- [glText](https://github.com/vallentin/glText) - Text rendering
- [FastNoiseLite](https://github.com/Auburn/FastNoiseLite) - Noise generation

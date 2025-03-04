#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

out vec2 texCoords;
out vec2 atlasCoords;
out vec3 normal;

in DATA {
  vec2 texCoords;
  vec2 atlasCoords;
  vec3 normal;
  vec3 pos;
  bool en;
} data_in[];

void main()
{
  for (int i = 0; i < 3; i++)
  {
    gl_Position = u_MVP * vec4(data_in[i].pos, 1.0);
    texCoords = data_in[i].texCoords;
    atlasCoords = data_in[i].atlasCoords;
    normal = data_in[i].normal;
    EmitVertex();
  }

  EndPrimitive();
}
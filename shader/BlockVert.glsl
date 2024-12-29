#version 330 core

layout (location = 0) in uint packedVertex; // 32 bits - 0: pos-z, 1: pos-y, 2: pos-x, 3-5: texture-x, 6: texture-y
layout (location = 1) in uvec3 instancePos;
layout (location = 2) in uvec2 instanceAtlasCoords;

uniform mat4 u_Projection, u_View;
uniform uvec2 u_ChunkPos;

out vec2 v_TexCoords;
out vec2 v_AtlasCoords;

vec2 unpackTextureCoords(uint data) {
  vec2 coords = vec2(0);

  coords.x = float((data >> 3u) & 7u);
  coords.y = float((data >> 6u) & 1u);

  return coords;
}

vec3 unpackPos(uint data)
{
  vec3 pos = vec3(0.5f, 0.5f, 0.5f);

  uint bit = data & 1u;
  if (bit == 1u)
  {
    pos.z = -0.5f;
  }
  bit = (data >> 1u) & 1u;
  if (bit == 1u)
  {
    pos.y = -0.5f;
  }
  bit = (data >> 2u) & 1u;
  if (bit == 1u)
  {
    pos.x = -0.5f;
  }

  return pos;
}

void main()
{
  const uint chunkSize = 16u;
  vec3 pos = vec3(instancePos + chunkSize * uvec3(u_ChunkPos.x, 0, u_ChunkPos.y));
  gl_Position = u_Projection * u_View * vec4(unpackPos(packedVertex) + pos, 1.0);

  v_TexCoords = unpackTextureCoords(packedVertex);
  v_AtlasCoords = instanceAtlasCoords;
}
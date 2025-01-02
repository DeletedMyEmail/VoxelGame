#version 330 core

// 32 bits - 0: pos-z, 1: pos-y, 2: pos-x, 3-5: texture-x, 6: texture-y, 7-9: normal, 10: disable
layout (location = 0) in uint packedVertex;
layout (location = 1) in uvec3 instancePos;
layout (location = 2) in uvec2 instanceAtlasCoords;

uniform uvec2 u_ChunkPos;

out DATA {
  vec2 texCoords;
  vec2 atlasCoords;
  vec3 normal;
  vec3 pos;
} data_out;

vec2 unpackTextureCoords(uint data);
vec3 unpackNormal(uint data);
vec3 unpackPos(uint data);

void main()
{
  const uint chunkSize = 16u;

  data_out.pos = vec3(instancePos + chunkSize * uvec3(u_ChunkPos.x, 0, u_ChunkPos.y)) + unpackPos(packedVertex);
  data_out.texCoords = unpackTextureCoords(packedVertex);
  data_out.atlasCoords = instanceAtlasCoords;
  data_out.normal = unpackNormal(packedVertex);
}

vec2 unpackTextureCoords(uint data) {
  vec2 coords = vec2(0);

  coords.x = float((data >> 3u) & 7u);
  coords.y = float((data >> 6u) & 1u);

  return coords;
}

vec3 unpackNormal(uint data) {
  uint x = (data >> 7u) & 7u;

  switch (x) {
    case 0u:
          return vec3(0.0f, 1.0f, 0.0f);
    case 1u:
          return vec3(0.0f, 0.0f, 1.0f);
    case 2u:
          return vec3(-1.0f, 0.0f, 0.0f);
    case 3u:
          return vec3(0.0f, -1.0f, 0.0f);
    case 4u:
          return vec3(1.0f, 0.0f, 0.0f);
    case 5u:
          return vec3(0.0f, 0.0f, -1.0f);
    default:
          return vec3(0);
  }
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
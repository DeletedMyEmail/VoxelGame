#version 330 core

// 32 bits - 0: pos-z, 1: pos-y, 2: pos-x, 3-5: texture-x, 6: texture-y, 7-9: normal, 10: atlas-x, 11-14: atlas-y, 15-18: pos-rel-to-chunk-x, 19-26: pos-rel-to-chunk-y, 27-30: pos-rel-to-chunk-z, 31: unused
layout (location = 0) in uint packedVertex;

uniform uvec2 u_ChunkPos;
uniform mat4 u_MVP;

out vec2 texCoords;
out vec2 atlasCoords;
out vec3 normal;

vec2 unpackTextureCoords(uint data);
vec3 unpackNormal(uint data);
vec3 unpackBlockMeshPos(uint data);
vec2 unpackAtlasCoords(uint data);
vec3 unpackRelPos(uint data);
vec3 calcPosition(uint data);

void main()
{
    gl_Position = u_MVP * vec4(calcPosition(packedVertex), 1);
    texCoords = unpackTextureCoords(packedVertex);
    atlasCoords = unpackAtlasCoords(packedVertex);
    normal = unpackNormal(packedVertex);
}

vec3 calcPosition(uint data)
{
    const uint chunkSize = 16u;
    return vec3(chunkSize * uvec3(u_ChunkPos.x, 0, u_ChunkPos.y)) + unpackBlockMeshPos(packedVertex) + unpackRelPos(packedVertex);
}

vec2 unpackAtlasCoords(uint data)
{
    vec2 coords = vec2(0);

    coords.x = float((data >> 10u) & 1u);
    coords.y = float((data >> 11u) & 15u);

    return coords;
}

vec3 unpackRelPos(uint data)
{
    vec3 pos = vec3(0);

    pos.x = float((data >> 15u) & 15u);
    pos.y = float((data >> 19u) & 255u);
    pos.z = float((data >> 27u) & 15u);

    return pos;
}

vec2 unpackTextureCoords(uint data)
{
    vec2 coords = vec2(0);

    coords.x = float((data >> 3u) & 7u);
    coords.y = float((data >> 6u) & 1u);

    return coords;
}

vec3 unpackNormal(uint data)
{
    uint x = (data >> 7u) & 7u;
    // TODO: branchless

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

vec3 unpackBlockMeshPos(uint data)
{
    vec3 pos;

    pos.x = data & 1u;
    pos.y = (data >> 1u) & 1u;
    pos.z = (data >> 2u) & 1u;

    return pos;
}
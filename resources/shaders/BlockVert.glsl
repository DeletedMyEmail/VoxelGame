#version 330 core

// 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
// face ------ x ------------ y ------------ z ------------ atlasX ----  atlasY ----  free ---------
// face: shift 28, F; x: shift 23, 1F; y: shift 18, 1F; z: shift 13, 1F; atlas x: shift 9, F; atlas y: shift 5, F, free: shift 0, 1F
layout (location = 0) in uint in_packedData;

uniform mat4 u_VP;
uniform vec3 u_chunkOffset;

out vec2 v_uv;
out vec3 v_normal;

const vec3 s_vertexPositions[36] = vec3[36](
    vec3(0.0f, 0.0f, 0.0f),// 0, back
    vec3(1.0f, 0.0f, 0.0f),// 1, back
    vec3(1.0f, 1.0f, 0.0f), // 2, back
    vec3(1.0f, 1.0f, 0.0f),  // 3, back
    vec3(0.0f, 1.0f, 0.0f),  // 4, back
    vec3(0.0f, 0.0f, 0.0f),  // 5, back

    vec3(0.0f, 0.0f, 1.0f), // 0, front
    vec3(0.0f, 1.0f, 1.0f), // 1, front
    vec3(1.0f, 1.0f, 1.0f), // 2, front
    vec3(1.0f, 1.0f, 1.0f), // 3, front
    vec3(1.0f, 0.0f, 1.0f), // 4, front
    vec3(0.0f, 0.0f, 1.0f), // 5, front

    vec3(0.0f, 0.0f, 1.0f), // 0, left
    vec3(0.0f, 0.0f, 0.0f), // 1, left
    vec3(0.0f, 1.0f, 0.0f), // 2, left
    vec3(0.0f, 1.0f, 0.0f), // 3, left
    vec3(0.0f, 1.0f, 1.0f), // 4, left
    vec3(0.0f, 0.0f, 1.0f), // 5, left

    vec3(1.0f, 0.0f, 0.0f), // 0, right
    vec3(1.0f, 0.0f, 1.0f), // 1, right
    vec3(1.0f, 1.0f, 1.0f), // 2, right
    vec3(1.0f, 1.0f, 1.0f), // 3, right
    vec3(1.0f, 1.0f, 0.0f), // 4, right
    vec3(1.0f, 0.0f, 0.0f), // 5, right

    vec3(0.0f, 0.0f, 0.0f), // 0, bottom
    vec3(1.0f, 0.0f, 0.0f), // 1, bottom
    vec3(1.0f, 0.0f, 1.0f), // 2, bottom
    vec3(1.0f, 0.0f, 1.0f), // 3, bottom
    vec3(0.0f, 0.0f, 1.0f), // 4, bottom
    vec3(0.0f, 0.0f, 0.0f), // 5, bottom

    vec3(0.0f, 1.0f, 0.0f), // 0, top
    vec3(1.0f, 1.0f, 0.0f), // 1, top
    vec3(1.0f, 1.0f, 1.0f), // 2, top
    vec3(1.0f, 1.0f, 1.0f), // 3, top
    vec3(0.0f, 1.0f, 1.0f), // 4, top
    vec3(0.0f, 1.0f, 0.0f)  // 5, top
);

const vec3 s_normals[6] = vec3[6](
    vec3(0.0f, 0.0f, -1.0f), // back
    vec3(0.0f, 0.0f, 1.0f),  // front
    vec3(-1.0f, 0.0f, 0.0f), // left
    vec3(1.0f, 0.0f, 0.0f),  // right
    vec3(0.0f, -1.0f, 0.0f), // bottom
    vec3(0.0f, 1.0f, 0.0f)   // top
);

void main()
{
    vec3 translation = u_chunkOffset;

    translation.x += float((in_packedData >> 23u) & 0x1Fu);
    translation.y += float((in_packedData >> 18u) & 0x1Fu);
    translation.z += float((in_packedData >> 13u) & 0x1Fu);

    uint faceIndex = (in_packedData >> 28u) & 0xFu;
    uint vertexIndex = faceIndex * 6u + uint(gl_VertexID) % 6u;
    vec3 vertexPos = s_vertexPositions[vertexIndex];
    gl_Position = u_VP * vec4(vertexPos + translation, 1.0f);

    v_normal = s_normals[faceIndex];

    uvec2 offset;
    if (faceIndex == 0u || faceIndex == 1u)
    offset = uvec2(vertexPos.x == 1.0f ? 0 : 1, vertexPos.y == 1.0f ? 0 : 1);
    else if (faceIndex == 2u || faceIndex == 3u)
    offset = uvec2(vertexPos.z == 1.0f ? 0 : 1, vertexPos.y == 1.0f ? 0 : 1);
    else if (faceIndex == 4u || faceIndex == 5u)
    offset = uvec2(vertexPos.z == 1.0f ? 0 : 1, vertexPos.x == 1.0f ? 0 : 1);

    v_uv = vec2(
        float(((in_packedData >> 9u) & 0xFu) + offset.x),
        float(((in_packedData >> 5u) & 0xFu) + offset.y)
    );
}
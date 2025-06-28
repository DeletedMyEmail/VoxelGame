#version 330 core

// 0x F(face) F(x) FF(y) F(z) F(atlas x) F(atlas y) F(unused)
layout (location = 0) in uint in_packedData;

uniform mat4 u_VP;
uniform vec3 u_chunkOffset;

out vec2 v_uv;
out vec3 v_normal;

const vec3 s_vertexPositions[36] = vec3[36](
    vec3(-0.5f, -0.5f, -0.5f),// 0, back
    vec3( 0.5f, -0.5f, -0.5f),// 1, back
    vec3( 0.5f,  0.5f, -0.5f), // 2, back
    vec3( 0.5f,  0.5f, -0.5f),  // 3, back
    vec3(-0.5f,  0.5f, -0.5f),  // 4, back
    vec3(-0.5f, -0.5f, -0.5f),  // 5, back

    vec3(-0.5f, -0.5f, 0.5f),   // 0, front
    vec3( 0.5f, -0.5f, 0.5f),   // 1, front
    vec3( 0.5f,  0.5f, 0.5f),   // 2, front
    vec3( 0.5f,  0.5f, 0.5f),   // 3, front
    vec3(-0.5f,  0.5f, 0.5f),   // 4, front
    vec3(-0.5f, -0.5f, 0.5f),   // 5, front

    vec3(-0.5f, -0.5f, -0.5f), // 0, left
    vec3(-0.5f, -0.5f,  0.5f), // 1, left
    vec3(-0.5f,  0.5f,  0.5f), // 2, left
    vec3(-0.5f,  0.5f,  0.5f), // 3, left
    vec3(-0.5f,  0.5f, -0.5f), // 4, left
    vec3(-0.5f, -0.5f, -0.5f), // 5, left

    vec3( 0.5f, -0.5f, -0.5f), // 0, right
    vec3( 0.5f, -0.5f,  0.5f), // 1, right
    vec3( 0.5f,  0.5f,  0.5f), // 2, right
    vec3( 0.5f,  0.5f,  0.5f), // 3, right
    vec3( 0.5f,  0.5f, -0.5f), // 4, right
    vec3( 0.5f, -0.5f, -0.5f), // 5, right

    vec3(-0.5f, -0.5f, -0.5f), // 0, bottom
    vec3( 0.5f, -0.5f, -0.5f), // 1, bottom
    vec3( 0.5f, -0.5f,  0.5f), // 2, bottom
    vec3( 0.5f, -0.5f,  0.5f), // 3, bottom
    vec3(-0.5f, -0.5f,  0.5f), // 4, bottom
    vec3(-0.5f, -0.5f, -0.5f), // 5, bottom

    vec3(-0.5f,  0.5f, -0.5f), // 0, top
    vec3( 0.5f,  0.5f, -0.5f), // 1, top
    vec3( 0.5f,  0.5f,  0.5f), // 2, top
    vec3( 0.5f,  0.5f,  0.5f), // 3, top
    vec3(-0.5f,  0.5f,  0.5f), // 4, top
    vec3(-0.5f,  0.5f, -0.5f)  // 5, top
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
    translation.x += float((in_packedData >> 24u) & 0xFu);
    translation.y += float((in_packedData >> 16u) & 0xFFu);
    translation.z += float((in_packedData >> 12u) & 0xFu);

    uint faceIndex = (in_packedData >> 28u) & 0xFu;
    uint vertexIndex = faceIndex * 6u + uint(gl_VertexID) % 6u;
    vec3 vertexPos = s_vertexPositions[vertexIndex];
    gl_Position = u_VP * vec4(vertexPos + translation, 1.0f);

    v_normal = s_normals[faceIndex];

    uvec2 offset;
    if (faceIndex == 0u || faceIndex == 1u)
        offset = uvec2(vertexPos.x == 0.5f ? 0 : 1, vertexPos.y == 0.5f ? 0 : 1);
    else if (faceIndex == 2u || faceIndex == 3u)
        offset = uvec2(vertexPos.z == 0.5f ? 0 : 1, vertexPos.y == 0.5f ? 0 : 1);
    else if (faceIndex == 4u || faceIndex == 5u)
        offset = uvec2(vertexPos.z == 0.5f ? 0 : 1, vertexPos.x == 0.5f ? 0 : 1);

    v_uv = vec2(
        float(((in_packedData >> 8u) & 0xFu) + offset.x),
        float(((in_packedData >> 4u) & 0xFu) + offset.y)
    );
}
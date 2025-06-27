#version 330 core

// 0x F(face) F(x) F(y) FF(z) FFF(texture?)
layout (location = 0) in uint in_packedData;

uniform mat4 u_VP;
uniform vec3 u_ChunkOffset;

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

void main()
{
    vec3 translation = u_ChunkOffset;
    translation.x += float((in_packedData >> 24u) & 0xFu);
    translation.y += float((in_packedData >> 20u) & 0xFu);
    translation.z += float((in_packedData >> 12u) & 0xFFu);

    uint vertexIndex = ((in_packedData >> 28u) & 0xFu) * 6u + uint(gl_VertexID) % 6u;
    vec3 pos = s_vertexPositions[vertexIndex] + translation;
    gl_Position = u_VP * vec4(pos, 1.0f);
}
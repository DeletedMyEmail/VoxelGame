#version 330 core

layout (location = 0) in uint in_packedData;

const uint s_faceMask = 0xFu, s_faceOffset = 28u;
const uint s_xPosMask = 0x1Fu, s_xPosOffset = 23u;
const uint s_yPosMask = 0x1Fu, s_yPosOffset = 18u;
const uint s_zPosMask = 0x1Fu, s_zPosOffset = 13u;
const uint s_atlasXMask = 0xFu, s_atlasXOffset = 9u;
const uint s_atlasYMask = 0xFu, s_atlasYOffset = 5u;

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
    vec3(0.0f, 0.0f, 1.0f), // 1, bottom
    vec3(1.0f, 0.0f, 1.0f), // 2, bottom
    vec3(1.0f, 0.0f, 1.0f), // 3, bottom
    vec3(1.0f, 0.0f, 0.0f), // 4, bottom
    vec3(0.0f, 0.0f, 0.0f), // 5, bottom

    vec3(0.0f, 1.0f, 0.0f), // 0, top
    vec3(1.0f, 1.0f, 0.0f), // 1, top
    vec3(1.0f, 1.0f, 1.0f), // 2, top
    vec3(1.0f, 1.0f, 1.0f), // 3, top
    vec3(0.0f, 1.0f, 1.0f), // 4, top
    vec3(0.0f, 1.0f, 0.0f)  // 5, top
);

const uint s_frontIndex = 0u;
const uint s_backIndex = 1u;
const uint s_leftIndex = 2u;
const uint s_rightIndex = 3u;
const uint s_bottomIndex = 4u;
const uint s_topIndex = 5u;

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

    translation.x += float((in_packedData >> s_xPosOffset) & s_xPosMask);
    translation.y += float((in_packedData >> s_yPosOffset) & s_yPosMask);
    translation.z += float((in_packedData >> s_zPosOffset) & s_zPosMask);

    uint faceIndex = (in_packedData >> s_faceOffset) & s_faceMask;
    uint vertexIndex = faceIndex * 6u + uint(gl_VertexID) % 6u;
    vec3 vertexPos = s_vertexPositions[vertexIndex];
    gl_Position = u_VP * vec4(vertexPos + translation, 1.0f);

    v_normal = s_normals[faceIndex];

    uvec2 faceOffset;
    if (faceIndex == s_frontIndex || faceIndex == s_backIndex)
    faceOffset = uvec2(vertexPos.x == 1.0f ? 0 : 1, vertexPos.y == 1.0f ? 0 : 1);
    else if (faceIndex == s_rightIndex || faceIndex == s_leftIndex)
    faceOffset = uvec2(vertexPos.z == 1.0f ? 0 : 1, vertexPos.y == 1.0f ? 0 : 1);
    else if (faceIndex == s_bottomIndex || faceIndex == s_topIndex)
    faceOffset = uvec2(vertexPos.z == 1.0f ? 0 : 1, vertexPos.x == 1.0f ? 0 : 1);

    v_uv = vec2(
        float(((in_packedData >> s_atlasXOffset) & s_atlasXMask) + faceOffset.x),
        float(((in_packedData >> s_atlasYOffset) & s_atlasYMask) + faceOffset.y)
    );
}
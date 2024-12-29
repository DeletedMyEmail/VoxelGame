#version 330 core

in vec2 v_TexCoords;

uniform sampler2D u_TextureSlot;
uniform uvec2 u_AtlasCoords;

layout(location = 0) out vec4 color;

void main()
{
    const float cellSize = 1.0f / 12.0f;

    vec2 coords = vec2(
        (v_TexCoords.x + float(u_AtlasCoords.x * 6u)) * cellSize,
        (v_TexCoords.y + float(u_AtlasCoords.y)) * cellSize
    );

    color = texture(u_TextureSlot, coords);
}
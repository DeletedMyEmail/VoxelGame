#version 330 core

in vec2 v_TexCoords;
in vec2 v_AtlasCoords;

uniform sampler2D u_TextureSlot;

layout(location = 0) out vec4 color;

void main()
{
    const float cellSize = 1.0f / 12.0f;

    vec2 coords = vec2(
        (v_TexCoords.x + v_AtlasCoords.x * 6.0f) * cellSize,
        (v_TexCoords.y + v_AtlasCoords.y) * cellSize
    );

    color = texture(u_TextureSlot, coords);
}
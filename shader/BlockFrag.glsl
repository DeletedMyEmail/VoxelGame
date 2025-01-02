#version 330 core

in vec2 texCoords;
in vec2 atlasCoords;
in vec3 normal;

uniform sampler2D u_TextureSlot;

layout(location = 0) out vec4 color;

vec4 getModelColor(vec2 atlasCoords, vec2 texCoords, sampler2D textureSlot) {
    const float cellSize = 1.0f / 12.0f;

    vec2 coords = vec2(
        (texCoords.x + atlasCoords.x * 6.0f) * cellSize,
        (texCoords.y + atlasCoords.y) * cellSize
    );

    return texture(textureSlot, coords);
}

void main()
{
    vec3 modelColor = getModelColor(atlasCoords, texCoords, u_TextureSlot).xyz;

    vec3 lightColor = vec3(0.8);
    vec3 lightSource = vec3(-1.0, 2.0, -1.0);
    vec3 lighting = vec3(0.3); // add ambient

    float diffuseStrength = max(0.0, dot(lightSource, normal));
    lighting += diffuseStrength * lightColor * 0.7; // add diffuse

    color = vec4(modelColor * lighting, 1.0f);
}
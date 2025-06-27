#version 330 core

const float s_atlasSize = 12.0f; // 16x16 texture atlas

in vec2 v_uv;
in vec3 v_normal;

uniform sampler2D u_textureSlot;

layout(location = 0) out vec4 color;

vec4 getTextureColor(vec2 uvCoords, sampler2D textureSlot);

void main()
{
    vec3 modelColor = getTextureColor(v_uv, u_textureSlot).xyz;

    vec3 lightColor = vec3(0.8);
    vec3 lightSource = vec3(-1.0, 2.0, -1.0);
    vec3 lighting = vec3(0.3); // add ambient

    float diffuseStrength = max(0.0, dot(lightSource, v_normal));
    lighting += diffuseStrength * lightColor * 0.7; // add diffuse

    color = vec4(modelColor * lighting, 1.0f);
}

vec4 getTextureColor(vec2 uvCoords, sampler2D textureSlot)
{
    const float cellSize = 1.0f / s_atlasSize;

    vec2 coords = vec2(
        uvCoords.x * cellSize,
        uvCoords.y * cellSize
    );

    return texture(textureSlot, coords);
}

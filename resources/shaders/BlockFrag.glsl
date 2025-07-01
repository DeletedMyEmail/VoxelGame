#version 330 core

const float s_atlasSize = 16.0f; // 16x16 texture atlas

in vec2 v_uv;
in vec3 v_normal;

uniform sampler2D u_textureSlot;
uniform vec3 u_exposure;

layout(location = 0) out vec4 out_color;

vec4 getTextureColor(vec2 uvCoords, sampler2D textureSlot);
vec3 Uncharted2Tonemap(vec3 x);
vec3 unchartedTonemapping(vec3 color);

void main()
{
    vec4 modelColor = getTextureColor(v_uv, u_textureSlot);
    modelColor.rgb = pow(modelColor.rgb, vec3(2.2)); // gamma correction!

    // lighting
    vec3 lightColor = vec3(0.8);
    vec3 lightSource = vec3(-1.0, 2.0, -1.0);
    vec3 lighting = vec3(0.3); // add ambient

    float diffuseStrength = max(0.0, dot(lightSource, v_normal));
    lighting += diffuseStrength * lightColor * 0.7; // add diffuse

    modelColor.rgb *= lighting;
    vec4 color = modelColor;

    // tonemapping
    color.rgb = unchartedTonemapping(color.rgb * u_exposure);
    color.rgb = pow(color.rgb, vec3(1/2.2)); //gama correction!

    out_color = color;
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

vec3 Uncharted2Tonemap(vec3 x)
{
    float Brightness = 0.28;
    x*= Brightness;
    float A = 0.28;
    float B = 0.29;
    float C = 0.10;
    float D = 0.2;
    float E = 0.025;
    float F = 0.35;
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 unchartedTonemapping(vec3 color)
{
    vec3 curr = Uncharted2Tonemap(color*4.7);
    color = curr/Uncharted2Tonemap(vec3(15.2));
    return color;
}
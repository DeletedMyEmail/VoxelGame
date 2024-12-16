#version 330 core

in vec2 v_TexCoords;

uniform sampler2D u_Texture;

layout(location = 0) out vec4 color;

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoords);
    color = texColor;
}
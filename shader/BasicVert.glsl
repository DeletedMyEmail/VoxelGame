#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoords;
layout (location = 2) in mat4 instanceMatrix;

uniform mat4 u_Projection;
uniform mat4 u_View;

out vec2 v_TexCoords;

void main()
{
    gl_Position = u_Projection * u_View * instanceMatrix * vec4(pos, 1.0);
    v_TexCoords = texCoords;
}
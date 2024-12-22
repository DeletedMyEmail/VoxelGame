#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoords;
layout (location = 2) in vec3 instancePos;

uniform mat4 u_Projection;
uniform mat4 u_View;

out vec2 v_TexCoords;

void main()
{
  gl_Position = u_Projection * u_View * vec4((pos + instancePos), 1.0);

  v_TexCoords = texCoords;
}
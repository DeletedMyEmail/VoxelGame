#version 330 core

layout (location = 0) in vec3 meshPos;
layout (location = 1) in vec4 color;

uniform mat4 u_VP;
uniform vec3 u_GlobalPosition;

out vec4 v_color;

void main()
{
    gl_Position = u_VP * vec4(u_GlobalPosition + meshPos, 1);
    v_color = color;
}
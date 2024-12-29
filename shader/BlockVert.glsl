#version 330 core

layout (location = 0) in uint data; // 32 bits - 0: pos-z, 1: pos-y, 2: pos-x, 3-5: texture-x, 6: texture-y

uniform mat4 u_Projection, u_View;

out vec2 v_TexCoords;

vec2 unpackTextureCoords(uint data) {
  vec2 coords = vec2(0);

  coords.x = float((data >> 3u) & 7u);
  coords.y = float((data >> 6u) & 1u);

  return coords;
}

vec3 unpackPos(uint data)
{
  vec3 pos = vec3(0.5f, 0.5f, 0.5f);

  uint bit = data & 1u;
  if (bit == 1u)
  {
    pos.z = -0.5f;
  }
  bit = (data >> 1u) & 1u;
  if (bit == 1u)
  {
    pos.y = -0.5f;
  }
  bit = (data >> 2u) & 1u;
  if (bit == 1u)
  {
    pos.x = -0.5f;
  }

  return pos;
}

void main()
{
  gl_Position = u_Projection * u_View * vec4(unpackPos(data), 1.0);
  v_TexCoords = unpackTextureCoords(data);
}
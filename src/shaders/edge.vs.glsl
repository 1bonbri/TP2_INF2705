#version 330 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 texCoords;

uniform mat4 mvp;

out vec2 TexCoords;

void main()
{
    TexCoords = texCoords;

    vec3 expandedPosition = position * 1.04;
    gl_Position = mvp * vec4(expandedPosition, 1.0);
}

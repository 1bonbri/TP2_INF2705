#version 330 core

in vec2 TexCoords;

uniform sampler2D diffuseSampler;

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(diffuseSampler, TexCoords);

    if (texColor.a < 0.5)
        discard;

    FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
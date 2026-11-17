#version 330 core

in ATTRIBS_GS_OUT
{
    vec2 texCoords;
} attribsIn;

uniform sampler2D diffuseSampler;

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(diffuseSampler, attribsIn.texCoords);
    if (texColor.a < 0.5)
        discard;

    vec3 color = mix(vec3(0.4, 0.3, 0.1), vec3(1.0), attribsIn.texCoords.y);
    FragColor = vec4(texColor.rgb * color, texColor.a);
}

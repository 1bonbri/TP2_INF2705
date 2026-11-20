#version 330 core

in ATTRIB_GS_OUT
{
    vec2 texCoord;
    vec4 color;
} attribIn;

out vec4 FragColor;

uniform sampler2D particleTexture;

void main()
{
    // Récupérer la couleur du texel dans la texture.
    vec4 texel = texture(particleTexture, attribIn.texCoord);

    // Éliminer les fragments presque transparents.
    if (texel.a < 0.02)
        discard;

    // Teinter la texture avec la couleur de la particule.
    FragColor = texel * attribIn.color;
}

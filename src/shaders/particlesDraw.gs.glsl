#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in ATTRIB_VS_OUT
{
    float zOrientation;
    vec4 color;
    vec2 size;
} attribIn[];

out ATTRIB_GS_OUT
{
    vec2 texCoord;
    vec4 color;
} attribOut;

uniform mat4 projection;

void main()
{
    // Position de la particule dans l'espace caméra.
    vec4 center = gl_in[0].gl_Position;

    // Calcul de la rotation autour de l'axe Z.
    float c = cos(attribIn[0].zOrientation);
    float s = sin(attribIn[0].zOrientation);

    // Demi-dimensions du carré.
    vec2 halfSize = attribIn[0].size * 0.5;

    // Coins du carré et coordonnées de texture correspondantes.
    vec2 corners[4] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 1.0, -1.0),
        vec2(-1.0,  1.0),
        vec2( 1.0,  1.0)
    );

    vec2 texCoords[4] = vec2[](
        vec2(0.0, 0.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(1.0, 1.0)
    );

    for (int i = 0; i < 4; ++i)
    {
        // Position locale du sommet.
        vec2 offset = corners[i] * halfSize;

        // Rotation du sommet autour de Z.
        vec2 rotatedOffset = vec2(
            c * offset.x - s * offset.y,
            s * offset.x + c * offset.y
        );

        // Position finale dans l'espace caméra.
        vec4 vertexPosition = center + vec4(rotatedOffset, 0.0, 0.0);

        // Projection vers l'espace écran.
        gl_Position = projection * vertexPosition;

        // Transmettre les attributs au fragment shader.
        attribOut.texCoord = texCoords[i];
        attribOut.color = attribIn[0].color;

        EmitVertex();
    }

    EndPrimitive();
}

#version 330 core

// Le shader transforme chaque point reçu en un brin d'herbe
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

out ATTRIBS_GS_OUT
{
    vec2 texCoords;
} attribsOut;


uniform mat4 mvp;


float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    // Définit la taille de base et fait varier la taille et les angles selon la position du brin.
    const float baseWidth = 0.5;
    const float varWidth = 0.1;
    const float baseHeight = 2.4;
    const float varHeight = 0.5;
    const float PI = 3.14159265;

    vec3 position = gl_in[0].gl_Position.xyz;
    float width = baseWidth + varWidth * rand(position.xz);
    float height = baseHeight + varHeight * rand(position.xz + vec2(1.0, 2.0));
    float angleY = 2.0 * PI * rand(position.xz + vec2(3.0, 4.0));
    float angleX = 0.1 * PI * rand(position.xz + vec2(5.0, 6.0));

    // Rotation autour de Y et inclinaison autour de X.
    mat3 rotationY = mat3(
        cos(angleY), 0.0, -sin(angleY),
        0.0, 1.0, 0.0,
        sin(angleY), 0.0, cos(angleY)
    );
    mat3 rotationX = mat3(
        1.0, 0.0, 0.0,
        0.0, cos(angleX), sin(angleX),
        0.0, -sin(angleX), cos(angleX)
    );
    mat3 rotation = rotationY * rotationX;

    // Définit les quatre coins du rectangle et leurs coordonnées dans la texture.
    vec3 offsets[4] = vec3[4](
        vec3(-width / 2.0, 0.0, 0.0),
        vec3(width / 2.0, 0.0, 0.0),
        vec3(-width / 2.0, height, 0.0),
        vec3(width / 2.0, height, 0.0)
    );
    vec2 texCoords[4] = vec2[4](
        vec2(0.0, 0.0), vec2(1.0, 0.0),
        vec2(0.0, 1.0), vec2(1.0, 1.0)
    );

    // Applique les rotations et la projection, puis émet les quatre sommets du rectangle.
    for (int i = 0; i < 4; i++)
    {
        attribsOut.texCoords = texCoords[i];
        gl_Position = mvp * vec4(position + rotation * offsets[i], 1.0);
        EmitVertex();
    }
    EndPrimitive();
}

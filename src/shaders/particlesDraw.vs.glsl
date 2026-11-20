#version 330 core

// Propriétés des particules provenant du VAO.
layout(location = 0) in vec3 position;
layout(location = 1) in float zOrientation;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 size;

out ATTRIB_VS_OUT
{
    float zOrientation;
    vec4 color;
    vec2 size;
} attribOut;

uniform mat4 modelView;

void main()
{
    // Transformer la position de la particule en coordonnées caméra.
    gl_Position = modelView * vec4(position, 1.0);

    // Transmettre les propriétés nécessaires au geometry shader.
    attribOut.zOrientation = zOrientation;
    attribOut.color = color;
    attribOut.size = size;
}

#version 400 core

layout(triangles, equal_spacing, point_mode) in;

void main()
{
    // Calcule les positions des points sur le sol où les brins d’herbe seront placés. 
    // gl_TessCoord = variable fournit par openGL, poids utilisé pour calculer la position de chaque point
    gl_Position = gl_TessCoord.x * gl_in[0].gl_Position
                + gl_TessCoord.y * gl_in[1].gl_Position
                + gl_TessCoord.z * gl_in[2].gl_Position;
}

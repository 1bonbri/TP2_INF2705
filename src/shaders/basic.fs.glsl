#version 330 core

// FS veut dire fragment shader
// A pour role de déterminer la couleur des fragments (pixel potentiel)
// Basic signifie shader simple qui n'a pas de matrice

// Déclare l'entrée de la couleur du sommet
in vec3 vertexColor;

// Déclare la sortie de la couleur du fragment
out vec4 fragmentColor;

void main()
{

    // Copie la couleur du sommet dans la sortie couleur de fragment
    fragmentColor = vec4(vertexColor, 1.0);
}

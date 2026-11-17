#version 400 core

// Chaque patch de sortie contient les trois sommets d'un triangle.
layout(vertices = 3) out;

uniform mat4 modelView;

void main()
{
    // Chaque exécution transmet son sommet au shader suivant. gl_InvocationID: 0, 1 ou 2
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // Calcule les niveaux de tessellation une seule fois par patch.
    if (gl_InvocationID == 0)
    {
        // Récupère les trois sommets du triangle.
        vec4 v0 = gl_in[0].gl_Position;
        vec4 v1 = gl_in[1].gl_Position;
        vec4 v2 = gl_in[2].gl_Position;

        // Set les niveaux de tessellation et les distances de transition.
        const float MIN_TESS = 2;
        const float MAX_TESS = 32;
        const float MIN_DIST = 10.0f;
        const float MAX_DIST = 40.0f;

        // Calcule le milieu de chacun des trois arêtes du triangle.
        vec4 midpoints[3] = vec4[3](
            (v1 + v2) * 0.5,
            (v2 + v0) * 0.5,
            (v0 + v1) * 0.5
        );

        // Détermine le nombre de divisions pour chaque arête.
        for (int i = 0; i < 3; i++)
        {
            // Place le milieu de l'arête dans le repère de la caméra.
            vec3 midpoint = vec3(modelView * midpoints[i]);

            // Ramène la distance entre 0 (proche) et 1 (loin).
            float factor = clamp((length(midpoint) - MIN_DIST) / (MAX_DIST - MIN_DIST), 0.0, 1.0);

            // Choisit un niveau élevé de près et faible de loin.
            float level = mix(MAX_TESS, MIN_TESS, factor);

            // Arrondit le niveau à un multiple de deux pour créer des paliers.
            gl_TessLevelOuter[i] = floor(level / 2.0) * 2.0;
        }

        // Utilise le plus grand niveau extérieur pour l'intérieur du triangle.
        gl_TessLevelInner[0] = max(gl_TessLevelOuter[0], max(gl_TessLevelOuter[1], gl_TessLevelOuter[2]));
    }
}

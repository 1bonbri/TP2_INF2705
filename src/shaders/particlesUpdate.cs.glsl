#version 430 core

layout(local_size_x = 64) in;

struct Particle
{
    vec3 position;
    float zOrientation;
    vec3 velocity;
    vec4 color;
    vec2 size;
    float timeToLive;
    float maxTimeToLive;
};

layout(std140, binding = 0) readonly restrict buffer ParticlesInputBlock
{
    Particle particles[];
} dataIn;

layout(std140, binding = 1) writeonly restrict buffer ParticlesOutputBlock
{
    Particle particles[];
} dataOut;

uniform float time;
uniform float deltaTime;
uniform vec3 emitterPosition;
uniform vec3 emitterDirection;
uniform uint nParticles;
uniform bool emitterActive;

// Random [0, 1]
float rand01(float seed)
{
    return fract(sin(dot(
        vec2(time * 100.0 + seed, gl_GlobalInvocationID.x),
        vec2(12.9898, 78.233)
    )) * 43758.5453);
}

void main()
{
    uint index = gl_GlobalInvocationID.x;

    // Évite de dépasser le nombre maximal de particules.
    if (index >= 64u)
        return;

    // Les particules qui ne sont pas encore actives ne sont pas mises à jour.
    if (index >= nParticles)
    {
        dataOut.particles[index] = dataIn.particles[index];
        return;
    }

    Particle p = dataIn.particles[index];

    // Initialisation de la particule si son temps de vie est écoulé.
    if (p.timeToLive <= 0.0)
    {
        // Ne pas créer de nouvelles particules lorsque la meule est immobile.
        if (!emitterActive)
        {
            dataOut.particles[index] = p;
            return;
        }

        p.position = emitterPosition;

        // Orientation aléatoire entre [0, 2pi].
        p.zOrientation = rand01(1.0) * 6.28318530718;

        // Direction de déplacement et mouvement naturel vers le haut.
        p.velocity = normalize(emitterDirection) * 0.3
                   + vec3(0.0, 0.2, 0.0);

        p.color = vec4(0.61, 0.52, 0.38, 0.0);

        // Taille initiale.
        p.size = vec2(1.2);

        // Temps de vie aléatoire entre [2.5, 4.0].
        p.maxTimeToLive = mix(2.5, 4.0, rand01(2.0));
        p.timeToLive = p.maxTimeToLive;
    }
    else
    {
        // Réduire le temps de vie.
        p.timeToLive -= deltaTime;

        // Mise à jour de la position par la méthode d'Euler.
        p.position += p.velocity * deltaTime;

        // Rotation constante autour de l'axe Z.
        p.zOrientation += 0.5 * deltaTime;

        // Progression de la particule : 0 = naissance, 1 = fin de vie.
        float age = 1.0 - p.timeToLive / p.maxTimeToLive;
        age = clamp(age, 0.0, 1.0);

        // La couleur devient progressivement blanche.
        p.color.rgb = mix(
            vec3(0.61, 0.52, 0.38),
            vec3(1.0),
            age
        );

        // Fade in et fade out de l'opacité.
        float fadeIn = smoothstep(0.0, 0.2, age);
        float fadeOut = 1.0 - smoothstep(0.8, 1.0, age);

        p.color.a = 0.2 * fadeIn * fadeOut;

        // Augmentation linéaire de la taille jusqu'à 2.5.
        float particleSize = mix(1.2, 2.5, age);
        p.size = vec2(particleSize);
    }

    // Écrire la particule mise à jour dans le buffer de sortie.
    dataOut.particles[index] = p;
}

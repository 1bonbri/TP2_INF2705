#version 330 core

#define MAX_SPOT_LIGHTS 8

in ATTRIBS_VS_OUT
{
    vec2 texCoords;
    vec3 normal;
    vec3 color;
} attribsIn;

in LIGHTS_VS_OUT
{
    vec3 obsPos;
    vec3 dirLightDir;
    
    vec3 spotLightsDir[MAX_SPOT_LIGHTS];
    vec3 spotLightsSpotDir[MAX_SPOT_LIGHTS];
} lightsIn;


struct Material
{
    vec3 emission;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirectionalLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    vec3 direction;
};

struct SpotLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    vec3 position;
    vec3 direction;
    float exponent;
    float openingAngle;
};

uniform int nSpotLights;

uniform vec3 globalAmbient;

layout (std140) uniform MaterialBlock
{
    Material mat;
};

layout (std140) uniform LightingBlock
{
    DirectionalLight dirLight;
    SpotLight spotLights[MAX_SPOT_LIGHTS];
};

uniform sampler2D diffuseSampler;

out vec4 FragColor;


float computeSpot(in float openingAngle, in float exponent, in vec3 spotDir, in vec3 lightDir, in vec3 normal)
{
    // TODO: Calcul de spotlight, l'algorithme classique d'OpenGL vu en classe (voir annexe).
    vec3 L = normalize(lightDir);
    vec3 axis = normalize(spotDir);

    // γ entre l'axe du spot et la direction vers la lumière.
    // L'angle d'ouverture est en degrés, comme dans les données de la scène.
    float cosGamma = dot(L, axis);
    float cosDelta = cos(radians(openingAngle));

    if (cosGamma <= cosDelta)
        return 0.0;

    return pow(cosGamma, exponent);
}

void main()
{
    vec4 texColor = texture(diffuseSampler, attribsIn.texCoords);
    if (texColor.a < 0.5)
        discard;

    vec3 N = normalize(attribsIn.normal);
    vec3 O = normalize(lightsIn.obsPos);

    // La texture diffuse module l'ambiante et la diffuse. Le spéculaire reste celui du matériel.
    vec3 ka = mat.ambient * texColor.rgb;
    vec3 kd = mat.diffuse * texColor.rgb;

    // TODO: Calcul d'illumination
    vec3 color = mat.emission + globalAmbient * ka;

    // Soleil : lumière directionnelle, même intensité partout.
    vec3 L = normalize(lightsIn.dirLightDir);
    float nDotL = max(dot(N, L), 0.0);

    color += dirLight.ambient * ka;
    color += dirLight.diffuse * kd * nDotL;
    if (nDotL > 0.0)
    {
        vec3 R = reflect(-L, N);
        color += dirLight.specular * mat.specular * pow(max(dot(R, O), 0.0), mat.shininess);
    }

    for (int i = 0; i < nSpotLights; i++)
    {
        // TODO: Calcul des spotlights
        vec3 toLight = lightsIn.spotLightsDir[i];
        float distance = length(toLight);

        // Utiliser un facteur d'atténuation. On peut utiliser smoothstep avec la distance
        // entre la surface illuminé et la source de lumière. Il devrait y avoir un effet de blending
        // entre 7 et 10 unitées.
        // Le facteur impacte la composante diffuse et spéculaire.
        // Pleine intensité jusqu'à 7 unités, éteinte à 10.
        float attenuation = 1.0 - smoothstep(7.0, 10.0, distance);
        float spotFactor = computeSpot(
            spotLights[i].openingAngle,
            spotLights[i].exponent,
            lightsIn.spotLightsSpotDir[i],
            toLight,
            N
        );

        L = normalize(toLight);
        nDotL = max(dot(N, L), 0.0);

        color += spotFactor * spotLights[i].ambient * ka;
        color += spotFactor * attenuation * spotLights[i].diffuse * kd * nDotL;

        if (nDotL > 0.0)
        {
            vec3 R = reflect(-L, N);
            float spec = pow(max(dot(R, O), 0.0), mat.shininess);
            color += spotFactor * attenuation * spotLights[i].specular * mat.specular * spec;
        }
    }

    FragColor = vec4(color, texColor.a);
}

#include "textures.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

Texture2D::Texture2D()
: m_id(0)
{

}

void Texture2D::load(const char* path)
{
    //Chargement de la texture en RAM
    int width, height, nChannels;
    stbi_set_flip_vertically_on_load(true);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    unsigned char* data = stbi_load(path, &width, &height, &nChannels, 0);
    if (data == NULL)
        std::cout << "Error loading texture \"" << path << "\": " << stbi_failure_reason() << std::endl;

    //Créer un handle opengl pour la manipuler
    //m_id addresse de la texture
    glGenTextures(1,&m_id);

    //Activer une unité de texture (comme un pointeur vers une texture)
    glActiveTexture(GL_TEXTURE0);
    
    //Bind l'unité de texture qu'on vient d'activer au handle créé
    glBindTexture(GL_TEXTURE_2D,m_id);

    GLenum format;
    nChannels == 3 ? (format = GL_RGB) : (format = GL_RGBA);
    
    glTexImage2D(
        GL_TEXTURE_2D, // type de texture
        0,  //niveau de réduction d'image mipmap 0 image de base
        format, //format de stockage de la texture en memoire gpu
        width,
        height,
        0,  // épaisseur de la bordure autour de la texture (0 on en veut pas)
        format, //format auquel les données seront stockées sur gpu
        GL_UNSIGNED_BYTE,   //format des données du array de pixels rgb
        data    //pointeur vers le array de pixels rgb
    );

    



    stbi_image_free(data);
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &m_id);
}

void Texture2D::setFiltering(GLenum filteringMode)
{
    //Filtre min/mag: Agrandissement et minimisation d'une texture pour 
    // la faire fit dans un endroit pas adapté à sa taille
    //Sélectionner la texture
    Texture2D::use();
    //Changer le mode de filtering entre nearest/linear
    //GL_NEAREST : prendre la couleur du texel le plus proche
    // GL_LINEAR : interpoler les couleurs des texels voisins
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER , filteringMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER , filteringMode);
}

void Texture2D::setWrap(GLenum wrapMode)
{
    Texture2D::use();
    //Wrap: quoi faire quand les coordonnées de la texture sortent de l'interval 0-1
    // 0-1 = Interval ou fit une texture en entier mais on peut demander a afficher une texture au dela de cet interval
    //GL_REPEAT : recommencer l’image lorsqu’on dépasse ses limites
    //GL_CLAMP_TO_EDGE : prolonger la couleur du bord de l’image
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode); //horizontal
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode); //vertical
}

void Texture2D::enableMipmap()
{
    //Le mipmap cest une version plus petite d'une texture. Si la texture est loin de la caméra,
    // on affiche la version plus petite. Réduit l'aliasing et améliore les performances
    Texture2D::use();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture2D::use()
{
    //Set la texture active
    glBindTexture(GL_TEXTURE_2D, m_id);
}

//
// Cubemap
//

TextureCubeMap::TextureCubeMap()
: m_id(0)
{

}

void TextureCubeMap::load(const char** pathes)
{
    const size_t N_TEXTURES = 6;
    // Même ordre que les images du cubemap dans main.cpp.
    const GLenum faceTargets[N_TEXTURES] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X, // Droite
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X, // Gauche
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, // Haut
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, // Bas
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, // Avant
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z  // Arrière
    };

    unsigned char* datas[N_TEXTURES];
    int widths[N_TEXTURES];
    int heights[N_TEXTURES];
    int nChannels[N_TEXTURES];
    stbi_set_flip_vertically_on_load(false);
    for (unsigned int i = 0; i < 6; i++)
    {
        datas[i] = stbi_load(pathes[i], &widths[i], &heights[i], &nChannels[i], 0);
        if (datas[i] == NULL)
            std::cout << "Error loading texture \"" << pathes[i] << "\": " << stbi_failure_reason() << std::endl;
    }

    // TODO: Chargement de la texture en mémoire graphique.
    //       Faites la configuration des min et mag filtering et du wrap S, T, R directement, ils
    //       ne seront pas modifiés ailleurs.

    //Créer un handle opengl pour la manipuler
    //m_id addresse de la texture
    glGenTextures(1,&m_id);

    //Activer une unité de texture (comme un pointeur vers une texture)
    glActiveTexture(GL_TEXTURE0);
    
    //Bind l'unité de texture qu'on vient d'activer au handle créé
    glBindTexture(GL_TEXTURE_CUBE_MAP,m_id);


    
    for (unsigned int i = 0; i < 6; i++)
    {
        // TODO
        GLenum format;
        nChannels[i] == 3 ? (format = GL_RGB) : (format = GL_RGBA);
        glTexImage2D(
            faceTargets[i], // Face du cube qui reçoit cette image
            0,  //niveau de réduction d'image mipmap 0 image de base
            format, //format de stockage de la texture en memoire gpu
            widths[i],
            heights[i],
            0,  // épaisseur de la bordure autour de la texture (0 on en veut pas)
            format, //format auquel les données seront stockées sur gpu
            GL_UNSIGNED_BYTE,   //format des données du array de pixels rgb
            datas[i]    //pointeur vers le array de pixels rgb
        );
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (unsigned int i = 0; i < 6; i++)
    {
        stbi_image_free(datas[i]);
    }
}

TextureCubeMap::~TextureCubeMap()
{
    // TODO: Libérer les ressources allouées.
    glDeleteTextures(1, &m_id);
}

void TextureCubeMap::use()
{
    // TODO: Met la texture active pour être utilisée dans les prochaines commandes de dessins.
    glBindTexture(GL_TEXTURE_CUBE_MAP,m_id);
}

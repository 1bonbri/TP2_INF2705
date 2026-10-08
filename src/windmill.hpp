#pragma once

#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>

#include "model.hpp"

// TODO: À ajouter dans votre classe actuelle.

class PhongShading;


class Windmill
{   
public:
    Windmill();
    
    void loadModels();
    
    void update(float deltaTime);
    
    void draw(glm::mat4& projView, glm::mat4& view); // À besoin de la matrice de vue séparément, pour la partie 3.

    
private:
    
    void sendMatrixToShaders(
        const glm::mat4& projView,
        glm::mat4& view,
        glm::mat4 model
    );
    void drawCompleteBlade(
        const glm::mat4& projView,
        glm::mat4& view,
        const glm::mat4& rotorCenterMatrix
    );

private:    
    // TODO: Adapter les paramètres des méthodes privée ici au besoin, surtout pour la partie 3.    
    Model blade_;
    Model bladeframe_;
    Model bladebeam_;
    Model mainbeam_;
    Model millstone_;
    Model roof_;
    Model walls_;
    
public:
    float windSpeed;
    float windAngle;

    float angularSpeed;
    float rotorAngle;
    
    float roofAngle;
    
    PhongShading* phongShadingShader = nullptr;

};

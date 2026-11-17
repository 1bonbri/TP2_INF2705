#include <cstddef>
#include <cstdint>

#include <array>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "happly.h"
#include <imgui/imgui.h>

#include <inf2705/OpenGLApplication.hpp>

#include "model.hpp"
#include "windmill.hpp"

// TODO: À ajouter et compléter dans votre projet.
#include <map>

#include "model_data.hpp"
#include "shaders.hpp"
#include "textures.hpp"
#include "uniform_buffer.hpp"
#include "shader_storage_buffer.hpp"



#define CHECK_GL_ERROR printGLError(__FILE__, __LINE__)

using namespace gl;
using namespace glm;

// Définition des structures pour la communication avec le shader. NE PAS MODIFIER.

struct Material
{
    glm::vec4 emission; // vec3, but padded
    glm::vec4 ambient;  // vec3, but padded
    glm::vec4 diffuse;  // vec3, but padded
    glm::vec3 specular;
    GLfloat shininess;
};

struct DirectionalLight
{
    glm::vec4 ambient;   // vec3, but padded
    glm::vec4 diffuse;   // vec3, but padded
    glm::vec4 specular;  // vec3, but padded    
    glm::vec4 direction; // vec3, but padded
};

struct SpotLight
{
    glm::vec4 ambient;   // vec3, but padded
    glm::vec4 diffuse;   // vec3, but padded
    glm::vec4 specular;  // vec3, but padded

    glm::vec4 position;  // vec3, but padded
    glm::vec3 direction;
    GLfloat exponent;
    GLfloat openingAngle;

    GLfloat padding[3];
};

// Matériels

Material defaultMat =
{
    {0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.7f, 0.7f, 0.7f},
    10.0f
};

Material grassMat =
{
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.8f, 0.8f, 0.8f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.05f, 0.05f, 0.05f},
    100.0f
};

Material bezierMat =
{
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f},
    1.0f
};

struct BezierCurve
{
    glm::vec3 p0;
    glm::vec3 c0;
    glm::vec3 c1;
    glm::vec3 p1;
};


BezierCurve curves[5] =
{
    {
        glm::vec3(-0.509, 2.403, 6.924),
        glm::vec3(2.412, 12.011, 20.396),
        glm::vec3(-23.731, 5.855, 25.199),
        glm::vec3(-25.732, 15.439, 13.561),
    },
    {
        glm::vec3(-25.732, 15.439, 13.561),
        glm::vec3(-27.069, 21.845, 5.781),
        glm::vec3(-4.807, 26.461, -10.716),
        glm::vec3(-7.574, 25.967, -12.861),
    },
    {
        glm::vec3(-7.574, 25.967, -12.861),
        glm::vec3(-21.299, 23.516, -23.495),
        glm::vec3(-18.956, 13.960, -36.813),
        glm::vec3(-3.324, 11.588, -33.743),
    },
    {
        glm::vec3(-3.324, 11.588, -33.743),
        glm::vec3(4.812, 10.354, -32.145),
        glm::vec3(8.408, 22.100, -7.555),
        glm::vec3(23.264, 6.067, -5.370),
    },
    {
        glm::vec3(23.264, 6.067, -5.370),
        glm::vec3(29.367, -0.518, -4.473),
        glm::vec3(2.412, 12.011, 20.396),
        glm::vec3(-0.088, 12.011, 17.896),
    },
};

// Partie 1
glm::vec3 evaluateBezier(const BezierCurve& curve, float t)
{
    float u = 1.0f - t;

    return
        u * u * u * curve.p0 +
        3.0f * u * u * t * curve.c0 +
        3.0f * u * t * t * curve.c1 +
        t * t * t * curve.p1;
}


// Partie 3
// Ne pas modifier
struct Particle
{
    glm::vec3 position;
    GLfloat zOrientation;
    glm::vec4 velocity; // vec3, but padded
    glm::vec4 color;
    glm::vec2 size;
    GLfloat timeToLive;
    GLfloat maxTimeToLive;
};


struct App : public OpenGLApplication
{
    App()
        : isDay_(true) // TODO: À ajouter.
        , cameraPosition_(0.f, 0.f, 0.f)
        , cameraOrientation_(0.f, 0.f)
        , currentScene_(0)
        , isMouseMotionEnabled_(false)
        , totalTime(0.0)       // Initialiser à 0
        , timerParticles_(0.0) // Initialiser à 0
        , nParticles_(0)       // Initialiser à 0

    {}

    ~App() override
    {
        glDeleteBuffers(1, &grassVBO_);
        glDeleteVertexArrays(1, &grassVAO_);
    }

    void init() override
    {
        // Le message expliquant les touches de clavier.
        setKeybindMessage(
            "ESC : quitter l'application." "\n"
            "T : changer de scène." "\n"
            "W : déplacer la caméra vers l'avant." "\n"
            "S : déplacer la caméra vers l'arrière." "\n"
            "A : déplacer la caméra vers la gauche." "\n"
            "D : déplacer la caméra vers la droite." "\n"
            "Q : déplacer la caméra vers le bas." "\n"
            "E : déplacer la caméra vers le haut." "\n"
            "Flèches : tourner la caméra." "\n"
            "Souris : tourner la caméra" "\n"
            "Espace : activer/désactiver la souris." "\n"
        );

        // Config de base.
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        phongShadingShader_.create();
        edgeEffectShader_.create();
        skyShader_.create();

        windmill_.phongShadingShader = &phongShadingShader_;

        grassTexture_.load("../textures/grass.jpg");
        fenceTexture_.load("../textures/fence.png");
        windmillTexture_.load("../textures/windmill.png");

        grassTexture_.setFiltering(GL_LINEAR);
        windmillTexture_.setFiltering(GL_LINEAR);
        fenceTexture_.setFiltering(GL_NEAREST);

        grassTexture_.enableMipmap();
        windmillTexture_.enableMipmap();

        fenceTexture_.setWrap(GL_REPEAT);
        grassTexture_.setWrap(GL_REPEAT);

        windmillTexture_.setWrap(GL_CLAMP_TO_EDGE);

        whiteTexture_.load("../textures/white.png");
        whiteTexture_.setFiltering(GL_NEAREST);

        glEnable(GL_STENCIL_TEST);
        glStencilMask(0x00);

        const char* pathes[] = {
            "../textures/skybox/Daylight Box_Right.bmp",
            "../textures/skybox/Daylight Box_Left.bmp",
            "../textures/skybox/Daylight Box_Top.bmp",
            "../textures/skybox/Daylight Box_Bottom.bmp",
            "../textures/skybox/Daylight Box_Front.bmp",
            "../textures/skybox/Daylight Box_Back.bmp",
        };

        const char* nightPathes[] = {
            "../textures/skyboxNight/right.png",
            "../textures/skyboxNight/left.png",
            "../textures/skyboxNight/top.png",
            "../textures/skyboxNight/bottom.png",
            "../textures/skyboxNight/front.png",
            "../textures/skyboxNight/back.png",
        };

        skyboxTexture_.load(pathes);
        skyboxNightTexture_.load(nightPathes);

        loadModels();

        material_.allocate(&defaultMat, sizeof(Material));
        material_.setBindingIndex(0);

        lightsData_.dirLight =
        {
            {0.2f, 0.2f, 0.2f, 0.0f},
            {1.0f, 1.0f, 1.0f, 0.0f},
            {0.5f, 0.5f, 0.5f, 0.0f},
            {0.5f, -1.0f, 0.5f, 0.0f}
        };

        lightsData_.spotLights[0].position = glm::vec4(-1.6, 0.64, -0.45, 0.0f);
        lightsData_.spotLights[0].direction = glm::vec3(-10, -1, 0);
        lightsData_.spotLights[0].exponent = 4.0f;
        lightsData_.spotLights[0].openingAngle = 30.f;

        lightsData_.spotLights[1].position = glm::vec4(-1.6, 0.64, 0.45, 0.0f);
        lightsData_.spotLights[1].direction = glm::vec3(-10, -1, 0);
        lightsData_.spotLights[1].exponent = 4.0f;
        lightsData_.spotLights[1].openingAngle = 30.f;

        lightsData_.spotLights[2].position = glm::vec4(1.6, 0.64, -0.45, 0.0f);
        lightsData_.spotLights[2].direction = glm::vec3(10, -1, 0);
        lightsData_.spotLights[2].exponent = 4.0f;
        lightsData_.spotLights[2].openingAngle = 60.f;

        toggleSpotlights();

        setLightingUniform();

        lights_.allocate(&lightsData_, sizeof(lightsData_));
        lights_.setBindingIndex(1);

        /*
        -----------------------------------------------------------------------------------
        TP3
        -----------------------------------------------------------------------------------
        */
        grassShader_.create();
        wheatTexture_.load("../textures/wheat_crop.png");
        wheatTexture_.setFiltering(GL_LINEAR);
        wheatTexture_.setWrap(GL_CLAMP_TO_EDGE);
        wheatTexture_.enableMipmap();
        initGrass();

        // Partie 1-2
        // TODO: Initialisation des meshes (béziers, patches)
        // Création du VAO et du VBO pour la spline Bézier
        glGenVertexArrays(1, &bezierVAO_);
        glGenBuffers(1, &bezierVBO_);

        glBindVertexArray(bezierVAO_);
        glBindBuffer(GL_ARRAY_BUFFER, bezierVBO_);

        // Réserver suffisamment d'espace pour le maximum de subdivisions.
        // 5 courbes avec 17 segments maximum.
        const unsigned int MAX_VERTICES = 5 * (17 + 1);

        glBufferData(
            GL_ARRAY_BUFFER,
            MAX_VERTICES * sizeof(glm::vec3),
            nullptr,
            GL_DYNAMIC_DRAW
        );

        // Attribut 0 : position
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(glm::vec3),
            (void*)0
        );

        glBindVertexArray(0);

        glEnable(GL_PROGRAM_POINT_SIZE); // pour être en mesure de modifier gl_PointSize dans les shaders

        updateBezierCurve();

        // Partie 3
        // TODO: Allocation des SSBO.
        //       Allouer suffisament d'espace pour le nombre maximal de particules.
        //       Seulement le buffer en entrée à besoin d'être initialisé à 0.
        //       Réfléchisser au type d'usage.

        // TODO: Créer un vao pour le dessin des particules et activer les attributs nécessaires.


        // TODO: Création des nouveaux shaders.


        // TODO: Initialisation de la nouvelle texture pour les particules.
        // "../textures/smoke.png"




        CHECK_GL_ERROR;
    }


    // Appelée à chaque trame. Le buffer swap est fait juste après.
    void drawFrame() override
    {
        // TODO TP3: Au besoin, ajouter la recharge de vos nouveaux shaders
   //       après if (ImGui::Button("Reload Shaders")).
        CHECK_GL_ERROR;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        ImGui::Begin("Scene Parameters");
        ImGui::Combo("Scene", &currentScene_, SCENE_NAMES, N_SCENE_NAMES);

        if (ImGui::Button("Reload Shaders"))
        {
            CHECK_GL_ERROR;
            edgeEffectShader_.reload();
            phongShadingShader_.reload();
            skyShader_.reload();
            grassShader_.reload();

            setLightingUniform();
            CHECK_GL_ERROR;
        }
        ImGui::End();

        switch (currentScene_)
        {
        case 0: sceneMain(); break;
        }
        CHECK_GL_ERROR;
    }


    // Contrôles de caméra et projection conservés du TP1.
    void onKeyPress(const sf::Event::KeyPressed& key) override
    {
        using enum sf::Keyboard::Key;
        switch (key.code)
        {
        case Escape:
            window_.close();
            break;
        case Space:
            isMouseMotionEnabled_ = !isMouseMotionEnabled_;
            if (isMouseMotionEnabled_)
            {
                window_.setMouseCursorGrabbed(true);
                window_.setMouseCursorVisible(false);
            }
            else
            {
                window_.setMouseCursorGrabbed(false);
                window_.setMouseCursorVisible(true);
            }
            break;
        case T:
            currentScene_ = ++currentScene_ < N_SCENE_NAMES ? currentScene_ : 0;
            break;
        default: break;
        }
    }

    void onResize(const sf::Event::Resized& event) override
    {}

    void onMouseMove(const sf::Event::MouseMoved& mouseDelta) override
    {
        if (!isMouseMotionEnabled_)
            return;

        const float MOUSE_SENSITIVITY = 0.1;
        float cameraMouvementX = mouseDelta.position.y * MOUSE_SENSITIVITY;
        float cameraMouvementY = mouseDelta.position.x * MOUSE_SENSITIVITY;
        cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;
    }

    void updateCameraInput()
    {
        if (!window_.hasFocus())
            return;

        if (isMouseMotionEnabled_)
        {
            sf::Vector2u windowSize = window_.getSize();
            sf::Vector2i windowHalfSize(windowSize.x / 2.0f, windowSize.y / 2.0f);
            sf::Mouse::setPosition(windowHalfSize, window_);
        }

        float cameraMouvementX = 0;
        float cameraMouvementY = 0;

        const float KEYBOARD_MOUSE_SENSITIVITY = 1.5f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            cameraMouvementX -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            cameraMouvementX += KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            cameraMouvementY -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            cameraMouvementY += KEYBOARD_MOUSE_SENSITIVITY;

        cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;

        // Keyboard input
        glm::vec3 positionOffset = glm::vec3(0.0);
        const float SPEED = 10.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            positionOffset.z -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            positionOffset.z += SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            positionOffset.x -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            positionOffset.x += SPEED;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
            positionOffset.y -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
            positionOffset.y += SPEED;

        positionOffset = glm::rotate(glm::mat4(1.0f), cameraOrientation_.y, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(positionOffset, 1);
        cameraPosition_ += positionOffset * glm::vec3(deltaTime_);
    }

    glm::mat4 getPerspectiveProjectionMatrix()
    {
        // getWindowAspect();

        mat4 perspectiveMatrix = perspective(radians(70.0f), getWindowAspect(), 0.1f, 300.0f);

        return perspectiveMatrix;
    }


    void loadModels()
    {
        windmill_.loadModels();
        fence_.load("../models/fence.ply");
        spotlight_.load("../models/spotlight.ply");
        skybox_.load("../models/skybox.ply");

        grass_.load(ground, sizeof(ground), planeElements, sizeof(planeElements));
    }
    void drawFences(glm::mat4& projView, glm::mat4& view)
    {
        const std::vector<glm::vec3> FENCES_POSITIONS =
        {
            // Moulin centré en x = 0, z = -10. Sol à y = -0.1.
            // Panneaux espacés de 2 unités.
            //       _______
            //       |     |
            //       |  M  |
            //       | | | |
            //       |_| |_|

            // Arrière : z = -18.
            {-7.0f, -0.1f, -18.0f}, //Un vecteur = Une barrière
            {-5.0f, -0.1f, -18.0f},
            {-3.0f, -0.1f, -18.0f},
            {-1.0f, -0.1f, -18.0f},
            { 1.0f, -0.1f, -18.0f},
            { 3.0f, -0.1f, -18.0f},
            { 5.0f, -0.1f, -18.0f},
            { 7.0f, -0.1f, -18.0f},

            // Côté gauche : x = -8.
            {-8.0f, -0.1f, -17.0f},
            {-8.0f, -0.1f, -15.0f},
            {-8.0f, -0.1f, -13.0f},
            {-8.0f, -0.1f, -11.0f},
            {-8.0f, -0.1f,  -9.0f},
            {-8.0f, -0.1f,  -7.0f},
            {-8.0f, -0.1f,  -5.0f},
            {-8.0f, -0.1f,  -3.0f},
            {-8.0f, -0.1f,  -1.0f},
            {-8.0f, -0.1f,   1.0f},

            // Côté droit : x = 8.
            {8.0f, -0.1f, -17.0f},
            {8.0f, -0.1f, -15.0f},
            {8.0f, -0.1f, -13.0f},
            {8.0f, -0.1f, -11.0f},
            {8.0f, -0.1f,  -9.0f},
            {8.0f, -0.1f,  -7.0f},
            {8.0f, -0.1f,  -5.0f},
            {8.0f, -0.1f,  -3.0f},
            {8.0f, -0.1f,  -1.0f},
            {8.0f, -0.1f,   1.0f},

            // Avant : z = 2, avec une ouverture entre x = -2 et x = 2.
            {-7.0f, -0.1f, 2.0f},
            {-5.0f, -0.1f, 2.0f},
            {-3.0f, -0.1f, 2.0f},
            { 3.0f, -0.1f, 2.0f},
            { 5.0f, -0.1f, 2.0f},
            { 7.0f, -0.1f, 2.0f},

            // Le passage s'arrête à z = -4, avant la façade située à z = -5.
            // Passage intérieur : rangée gauche, x = -2.
            {-2.0f, -0.1f,  1.0f},
            {-2.0f, -0.1f, -1.0f},
            {-2.0f, -0.1f, -3.0f},

            // Passage intérieur : rangée droite, x = 2.
            {2.0f, -0.1f,  1.0f},
            {2.0f, -0.1f, -1.0f},
            {2.0f, -0.1f, -3.0f}
        };

        const std::vector<float> FENCES_ANGLES =
        {
            // Angles en radians, dans le même ordre que FENCES_POSITIONS.

            // Arrière : 8 panneaux à 90°.
            glm::radians(90.0f),
            glm::radians(90.0f),
            glm::radians(90.0f),
            glm::radians(90.0f),
            glm::radians(90.0f),
            glm::radians(90.0f),
            glm::radians(90.0f),
            glm::radians(90.0f),

            // Côté gauche : 10 panneaux à 0°.
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,

            // Côté droit : 10 panneaux à 0°.
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,

            // Avant : 6 panneaux à 90°.
            glm::radians(90.0f),
            glm::radians(90.0f),
            glm::radians(90.0f),
            glm::radians(90.0f),
            glm::radians(90.0f),
            glm::radians(90.0f),

            // Passage intérieur gauche : 3 panneaux à 0°.
            0.0f,
            0.0f,
            0.0f,

            // Passage intérieur droit : 3 panneaux à 0°.
            0.0f,
            0.0f,
            0.0f
        };
        // Active le shader Phong pour dessiner les clôtures avec leur texture.
        phongShadingShader_.use();
        setMaterial(defaultMat);
        glActiveTexture(GL_TEXTURE0);
        fenceTexture_.use();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDepthMask(GL_FALSE);

        constexpr unsigned int N_FENCES = 40;
        std::map<float, unsigned int> sorted;
        for (unsigned int i = 0; i < N_FENCES; i++)
        {
            glm::vec4 viewPosition = view * glm::vec4(FENCES_POSITIONS[i], 1.0f);
            float distance = glm::length(glm::vec3(viewPosition));

            while (sorted.count(distance) != 0)
                distance = std::nextafter(distance, INFINITY);

            sorted.insert({ distance, i });
        }

        // Préparer le stencil pour marquer les pixels occupés par les clôtures.
        // Ces valeurs serviront à distinguer l'intérieur de la clôture de son contour.
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF); //Autorise les ecritures dans le stencil
        glStencilFunc(GL_ALWAYS, 1, 0xFF); //Le test stencil reussit toujours
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); //Ecrire 1 lorsque le test de profondeur reussit

        // Dessiner les clôtures normalement.
        // Cette passe sert aussi à enregistrer leur forme dans le stencil.
        phongShadingShader_.use();
        setMaterial(defaultMat);
        fenceTexture_.use();

        for (std::map<float, unsigned int>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            unsigned int i = it->second;
            glm::mat4 model = glm::translate(glm::mat4(1.0f), FENCES_POSITIONS[i]);
            model = glm::rotate(model, FENCES_ANGLES[i], glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(2.0f));

            glm::mat4 mvp = projView * model;

            phongShadingShader_.setMatrices(mvp, view, model);
            fence_.draw();

            model = model
                * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.25f, 0.0f))
                * glm::scale(glm::mat4(1.0f), glm::vec3(1.05f))
                * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.25f, 0.0f));
        }

        glStencilMask(0x00); // Empêcher toute nouvelle écriture dans le stencil.

        // Dessiner uniquement là où la valeur du stencil est différente de 1.
        // Cela permet de dessiner le modèle agrandi à l'extérieur de la clôture
        // originale, sans recouvrir sa surface déjà dessinée.
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

        // Ne modifier aucune valeur du stencil pendant cette passe.
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        // Le contour est opaque : désactiver le mélange alpha pour cette passe.
        glDisable(GL_BLEND);

        // Utiliser le shader spécialisé qui dessine le contour noir.
        edgeEffectShader_.use();


        for (std::map<float, unsigned int>::reverse_iterator it = sorted.rbegin();
            it != sorted.rend(); ++it)
        {
            unsigned int i = it->second;

            glm::mat4 model = glm::translate(
                glm::mat4(1.0f), FENCES_POSITIONS[i]);

            model = glm::rotate(
                model, FENCES_ANGLES[i], glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(2.0f));

            glm::mat4 mvp = projView * model;

            // Envoyer la matrice au shader de contour.
            glUniformMatrix4fv(
                edgeEffectShader_.mvpULoc,
                1,
                GL_FALSE,
                glm::value_ptr(mvp));


            // Dessiner le modèle agrandi.
            // Le test stencil empêche de dessiner les fragments à l'intérieur
            // de la clôture originale : seule la partie extérieure reste visible.
            fence_.draw();
        }

        glStencilMask(0xFF); // Réautoriser les écritures dans le stencil pour les prochains rendus.
        glDisable(GL_STENCIL_TEST); // Désactiver le test stencil.

        // Restaurer les paramètres pour les objets opaques.
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ZERO);
        glEnable(GL_CULL_FACE);
    }

    void drawGround(glm::mat4& projView, glm::mat4& view)
    {
        setMaterial(grassMat);
        // Modele 3D contenu dans grass.ply taille 1x1
        // Il faut baisser ses coordonnées dans le monde de 0.1 on fait ca à partir de la matrice modele
        static const vec3 translationVector = vec3(0.0f, -0.1f, 0.0f);
        static const vec3 scalingVector = vec3(50.0f, 1.0f, 50.0f);
        static const mat4 modelMatrix = scale(translate(mat4(1.0f), translationVector), scalingVector);

        mat4 mvp = projView * modelMatrix;

        // setMatrices attend une matrice de modèle modifiable.
        mat4 model = modelMatrix;
        phongShadingShader_.setMatrices(mvp, view, model);
        grassTexture_.use();
        grass_.draw();

    }

    void initGrass()
    {
        // Division : 30 carrés de 5 x 5 unités par côté.
        std::vector<glm::vec3> vertices;
        for (float x = -25.0f; x < 25.0f; x += 5.0f)
        {
            if (x >= -10.0f && x < 10.0f)
                continue;

            for (float z = -25.0f; z < 25.0f; z += 5.0f)
            {
                // Coins du carré.
                glm::vec3 v0(x, -0.1f, z);
                glm::vec3 v1(x + 5.0f, -0.1f, z);
                glm::vec3 v2(x + 5.0f, -0.1f, z + 5.0f);
                glm::vec3 v3(x, -0.1f, z + 5.0f);

                vertices.push_back(v0);
                vertices.push_back(v2);
                vertices.push_back(v1);
                vertices.push_back(v0);
                vertices.push_back(v3);
                vertices.push_back(v2);
            }
        }

        grassVertexCount_ = static_cast<GLsizei>(vertices.size());
        glGenVertexArrays(1, &grassVAO_);
        glGenBuffers(1, &grassVBO_);
        glBindVertexArray(grassVAO_);
        glBindBuffer(GL_ARRAY_BUFFER, grassVBO_);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glPatchParameteri(GL_PATCH_VERTICES, 3);
    }

    void drawGrass(glm::mat4& projView, glm::mat4& view)
    {
        grassShader_.use();
        glUniformMatrix4fv(grassShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(projView));
        glUniformMatrix4fv(grassShader_.modelViewULoc, 1, GL_FALSE, glm::value_ptr(view));
        glActiveTexture(GL_TEXTURE0);
        wheatTexture_.use();

        glDisable(GL_CULL_FACE);
        glBindVertexArray(grassVAO_);
        glDrawArrays(GL_PATCHES, 0, grassVertexCount_);
        glBindVertexArray(0);
        glEnable(GL_CULL_FACE);
    }

    // Le modèle du projecteur est aligné sur -Y et n'a pas de normales.
    void drawSpotlights(glm::mat4& projView, glm::mat4& view)
    {
        phongShadingShader_.use();
        setMaterial(defaultMat);
        whiteTexture_.use();

        const vec3 modelAxis(0.0f, -1.0f, 0.0f);

        for (unsigned int i = 0; i < N_SPOTLIGHTS; ++i)
        {
            vec3 position = vec3(lightsData_.spotLights[i].position);
            vec3 direction = normalize(lightsData_.spotLights[i].direction);

            mat4 model = translate(mat4(1.0f), position);
            float cosine = dot(modelAxis, direction);
            if (cosine < 1)
            {
                vec3 axis = cross(modelAxis, direction);
                model = rotate(model, acos(clamp(cosine, -1.0f, 1.0f)), normalize(axis));
            }

            mat4 mvp = projView * model;
            phongShadingShader_.setMatrices(mvp, view, model);
            spotlight_.draw();
        }
    }

    // Solution
    glm::mat4 getViewMatrix()
    {
        //glm::rotate(matrice, angle, axe);
        mat4 viewMatrix(1.0f);

        viewMatrix = rotate(viewMatrix, -cameraOrientation_.x, vec3(1.0, 0.0, 0.0));
        viewMatrix = rotate(viewMatrix, -cameraOrientation_.y, vec3(0.0, 1.0, 0.0));


        vec3 translationVector = vec3(-cameraPosition_.x, -cameraPosition_.y, -cameraPosition_.z);

        viewMatrix = translate(viewMatrix, translationVector);


        return viewMatrix;
    }

    void setLightingUniform()
    {
        phongShadingShader_.use();
        glUniform1i(phongShadingShader_.nSpotLightsULoc, 3);

        float ambientIntensity = 0.05;
        glUniform3f(phongShadingShader_.globalAmbientULoc, ambientIntensity, ambientIntensity, ambientIntensity);
    }

    void toggleSun()
    {
        if (isDay_)
        {
            lightsData_.dirLight.ambient = glm::vec4(0.2f, 0.2f, 0.2f, 0.0f);
            lightsData_.dirLight.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
            lightsData_.dirLight.specular = glm::vec4(0.5f, 0.5f, 0.5f, 0.0f);
        }
        else
        {
            lightsData_.dirLight.ambient = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            lightsData_.dirLight.diffuse = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            lightsData_.dirLight.specular = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }

    void toggleSpotlights()
    {
        if (isDay_)
        {
            for (unsigned int i = 0; i < N_SPOTLIGHTS; i++)
            {
                lightsData_.spotLights[i].ambient = glm::vec4(glm::vec3(0.0f), 0.0f);
                lightsData_.spotLights[i].diffuse = glm::vec4(glm::vec3(0.0f), 0.0f);
                lightsData_.spotLights[i].specular = glm::vec4(glm::vec3(0.0f), 0.0f);
            }
        }
        else
        {
            for (unsigned int i = 0; i < N_SPOTLIGHTS; i++)
            {
                lightsData_.spotLights[i].ambient = glm::vec4(glm::vec3(0.02f), 0.0f);
                lightsData_.spotLights[i].diffuse = glm::vec4(glm::vec3(0.8f), 0.0f);
                lightsData_.spotLights[i].specular = glm::vec4(glm::vec3(0.4f), 0.0f);
            }
        }
    }

    // Le squelette appelle toggleStreetlight(), mais définit toggleSpotlights().
    void toggleStreetlight()
    {
        toggleSpotlights();
    }

    void setMaterial(Material& mat)
    {
        material_.updateData(&mat, 0, sizeof(Material));
    }
    void updateBezierCurve()
    {
        bezierVertices_.clear();
        bezierDistances_.clear();

        unsigned int divisions = bezierNPoints + 1;

        float totalDistance = 0.0f;
        glm::vec3 previousPoint;

        for (int i = 0; i < 5; i++)
        {
            for (unsigned int j = 0; j <= divisions; j++)
            {
                float t = (float)j / divisions;

                glm::vec3 point = evaluateBezier(curves[i], t);

                if (!bezierVertices_.empty())
                {
                    totalDistance += glm::length(point - previousPoint);
                }

                bezierVertices_.push_back(point);
                bezierDistances_.push_back(totalDistance);

                previousPoint = point;
            }
        }

        bezierTotalLength_ = totalDistance;

        glBindBuffer(GL_ARRAY_BUFFER, bezierVBO_);

        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            bezierVertices_.size() * sizeof(glm::vec3),
            bezierVertices_.data()
        );

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void drawBezierCurve(glm::mat4& projView, glm::mat4& view)
    {
        phongShadingShader_.use();

        setMaterial(bezierMat);
        whiteTexture_.use();

        glm::mat4 model(1.0f);
        glm::mat4 mvp = projView * model;

        phongShadingShader_.setMatrices(mvp, view, model);

        glBindVertexArray(bezierVAO_);

        unsigned int verticesPerCurve = bezierNPoints + 2;

        glLineWidth(3.0f);

        for (int i = 0; i < 5; i++)
        {
            glDrawArrays(
                GL_LINE_STRIP,
                i * verticesPerCurve,
                verticesPerCurve
            );
        }

        glBindVertexArray(0);
    }
   


    void sceneMain()
    {
        ImGui::Begin("Scene Parameters");
        if (ImGui::Button("Toggle Day/Night"))
        {
            isDay_ = !isDay_;
            toggleSun();
            toggleStreetlight();
            lights_.updateData(&lightsData_, 0, sizeof(DirectionalLight) + N_SPOTLIGHTS * sizeof(SpotLight));
        }


        //TODO: à Ajouter
        ImGui::SliderInt("Bezier Number Of Points", (int*)&bezierNPoints, 0, 16);
        if (ImGui::Button("Animate Camera"))
        {
            isAnimatingCamera = true;
            cameraAnimation = 0.0f;
            cameraMode = 1;
        }

        ImGui::SliderFloat("Wind Speed", &windmill_.windSpeed, 0.0f, 20.0f, "%.2f m/s");
        ImGui::SliderFloat("Wind Angle", &windmill_.windAngle, -M_PI, M_PI, "%.2f°");
        ImGui::End();

        updateCameraInput();
        windmill_.update(deltaTime_);

        skyShader_.use();
        glm::mat4 view = getViewMatrix();
        glm::mat4 proj = getPerspectiveProjectionMatrix();
        glm::mat4 projView = proj * view;

        // Garder la rotation de la caméra, retirer sa translation.
        glm::mat4 skyView = glm::mat4(glm::mat3(view));
        glm::mat4 skyMvp = proj * skyView;

        glUniformMatrix4fv(
            skyShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(skyMvp)
        );

        // Le samplerCube utilise l'unité 0 par défaut.
        glActiveTexture(GL_TEXTURE0);
        if (isDay_)
            skyboxTexture_.use();
        else
            skyboxNightTexture_.use();

        // Dessiner l'intérieur du cube comme arrière-plan.
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        skybox_.draw();

        // Restaurer les paramètres pour les autres objets.
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        phongShadingShader_.use();
        setMaterial(defaultMat);

        windmillTexture_.use();
        windmill_.draw(projView, view);

        drawGround(projView, view);
        drawGrass(projView, view);

        drawSpotlights(projView, view);

        // Dessiner les clôtures transparentes après les objets opaques.
        drawFences(projView, view);



        /*
        ---------------------------------------------------------------------------------------------------------------------------------------------------------
		TP3
		---------------------------------------------------------------------------------------------------------------------------------------------------------
        */

        // Partie 1-2

        // ...

        if (isAnimatingCamera)
        {
            if (cameraAnimation < 5)
            {
                // TODO: Animation de la caméra
                // cameraPosition_ = ...

                // Avancement de l'animation entre 0 et 5 secondes.
                cameraAnimation += deltaTime_ / 3.0f;

                // Limiter le temps à 1 pour éviter de dépasser la fin.
                float animationTime = glm::clamp(cameraAnimation / 5.0f, 0.0f, 1.0f);

                // Distance totale parcourue sur la spline.
                float currentDistance = animationTime * bezierTotalLength_;

                // Trouver les sommets qui encadrent la distance actuelle.
                auto it = std::lower_bound(
                    bezierDistances_.begin(),
                    bezierDistances_.end(),
                    currentDistance
                );

                if (it != bezierDistances_.end())
                {
                    unsigned int index = std::distance(
                        bezierDistances_.begin(), it
                    );

                    if (index == 0)
                    {
                        cameraPosition_ = bezierVertices_[0];
                    }
                    else
                    {
                        float d0 = bezierDistances_[index - 1];
                        float d1 = bezierDistances_[index];

                        float t = (d1 > d0)
                            ? (currentDistance - d0) / (d1 - d0)
                            : 0.0f;

                        cameraPosition_ = glm::mix(
                            bezierVertices_[index - 1],
                            bezierVertices_[index],
                            t
                        );
                    }
                }
                else
                {
                    cameraPosition_ = bezierVertices_.back();
                }

                // Toujours regarder le moulin.
                glm::vec3 distance = glm::vec3(0, 2, -10) - cameraPosition_;

                float horizontalDistance = sqrt(
                    distance.x * distance.x + distance.z * distance.z
                );

                cameraOrientation_.y = atan2(-distance.x, -distance.z);
                cameraOrientation_.x = atan2(distance.y, horizontalDistance);
      
            }
            else
            {
                // Remise à 0 de l'orientation
                glm::vec3 distance = glm::vec3(0, 2, -10) - cameraPosition_;

                float horizontalDistance = sqrt(distance.x * distance.x + distance.z * distance.z);

                cameraOrientation_.y = atan2(-distance.x, -distance.z);
                cameraOrientation_.x = atan2(distance.y, horizontalDistance);

                cameraAnimation = 0.f;
                isAnimatingCamera = false;
                cameraMode = 0;
            }
        }

        // ...

        bool hasNumberOfSidesChanged = bezierNPoints != oldBezierNPoints;
        if (hasNumberOfSidesChanged)
        {
            oldBezierNPoints = bezierNPoints;

            // TODO: Calcul et mise à jour de la courbe
            updateBezierCurve();
        }

        // TODO: Dessin de la courbe
        drawBezierCurve(projView, view);


        // ...

        // Partie 3
        // TODO: Attention à l'endroit où vous faites votre dessin, la texture des particules est transparente.

        // Particles    
        totalTime += deltaTime_;
        timerParticles_ += deltaTime_;
        const float particlesSpawnInterval = 0.2f;

        unsigned int particlesToAdd = timerParticles_ / particlesSpawnInterval;
        timerParticles_ -= particlesToAdd * particlesSpawnInterval;

        nParticles_ += particlesToAdd;
        if (nParticles_ > MAX_PARTICLES_)
            nParticles_ = MAX_PARTICLES_;

        // Particles update

        // TODO: Mise à jour des données à l'aide du compute shader
        //       Envoyer vos uniforms.

        // Utiliser la position de la meule pour calculer la nouvelle position et direction d'émission de particule.
        // glm::vec3 emitterPos = vec3(0.0, 0.0, 0.0);
        // glm::vec3 emitterDir = vec3(1, 0, 0);

        // TODO: Configurer les buffers d'entrée et de sortie.

        // TODO: Envois de la commande de calcul.
        //       Pas besoin d'optimiser le nombre de work group vs la taille local (dans le shader).


        // Particles draw

        // TODO: Dessin des particules. Utiliser le nombre de particules actuellement utilisées.
        //       Utiliser la texture et envoyer vos uniforms.
        //       Il sera nécessaire de spécifier les entrée en spécifiant le buffer d'entrée.
        //       Activer le blending et restaurer l'état du contexte modifié.

        // TODO: Interchanger les deux buffers, celui en entrée devient la sortie, et vice versa.

    }

private:
    // TODO: À ajouter.

    // Shaders
    EdgeEffect edgeEffectShader_;
    PhongShading phongShadingShader_;
    Sky skyShader_;
    GrassShader grassShader_;

    // Textures
    Texture2D grassTexture_;
    Texture2D wheatTexture_;
    Texture2D fenceTexture_;
    Texture2D windmillTexture_;
    Texture2D whiteTexture_;
    TextureCubeMap skyboxTexture_;
    TextureCubeMap skyboxNightTexture_;

    // Uniform buffers
    UniformBuffer material_;
    UniformBuffer lights_;


    struct {
        DirectionalLight dirLight;
        SpotLight spotLights[8];
    } lightsData_{};

    bool isDay_;

    Model grass_;
    Model fence_;
    Model spotlight_;
    Model skybox_;

    Windmill windmill_;

    glm::vec3 cameraPosition_;
    glm::vec2 cameraOrientation_;

    static constexpr unsigned int N_SPOTLIGHTS = 3;

    // Imgui var
    const char* const SCENE_NAMES[1] = {
        "Main scene"
    };
    const int N_SCENE_NAMES = sizeof(SCENE_NAMES) / sizeof(SCENE_NAMES[0]);
    int currentScene_;

    bool isMouseMotionEnabled_;
    /*
       ---------------------------------------------------------------------------------------------------------------------------------------------------------
       TP3
       ---------------------------------------------------------------------------------------------------------------------------------------------------------
    */
    // TODO: Ajouter ces attributs
    unsigned int bezierNPoints = 3; // Nombre de points supplémentaires sur la courbe, 0 est une ligne droite
    unsigned int oldBezierNPoints = 0;

    int cameraMode = 0;
    float cameraAnimation = 0.f;
    bool isAnimatingCamera = false;
    // Partie 1-2
    // TODO: Ajouter les attributs de vbo, ebo, vao nécessaire
    // Partie 1 : Bézier
    GLuint bezierVAO_;
    GLuint bezierVBO_;

    std::vector<glm::vec3> bezierVertices_;
    std::vector<float> bezierDistances_;
    float bezierTotalLength_ = 0.0f;

    GLuint grassVAO_ = 0;
    GLuint grassVBO_ = 0;
    GLsizei grassVertexCount_ = 0;

    // Partie 3
    // TODO: Attributs supplémentaires
    GLuint vaoParticles_;

    float totalTime;
    float timerParticles_;

    static const unsigned int MAX_PARTICLES_ = 64;
    unsigned int nParticles_;

    // TODO: Ajouter vos shaders
    // TODO: Ajouter la texture de particule

    // Ssbo
    ShaderStorageBuffer particles_[2];

};


int main(int argc, char* argv[])
{
    WindowSettings settings = {};
    settings.fps = 60;
    settings.context.depthBits = 24;
    settings.context.stencilBits = 8;
    settings.context.antiAliasingLevel = 4;
    settings.context.majorVersion = 4;
    settings.context.minorVersion = 3;
    settings.context.attributeFlags = sf::ContextSettings::Attribute::Core;

    App app;
    app.run(argc, argv, "Tp3", settings);
}

#include "shaders.hpp"

#include <glm/gtc/type_ptr.hpp>


void EdgeEffect::load()
{
    const char* VERTEX_SRC_PATH = "./shaders/edge.vs.glsl";
    const char* FRAGMENT_SRC_PATH = "./shaders/edge.fs.glsl";
    
    name_ = "EdgeEffect";
    loadShaderSource(GL_VERTEX_SHADER, VERTEX_SRC_PATH);
    loadShaderSource(GL_FRAGMENT_SHADER, FRAGMENT_SRC_PATH);
    link();
}

void EdgeEffect::getAllUniformLocations()
{
    mvpULoc = glGetUniformLocation(id_, "mvp");
}


void Sky::load()
{
    const char* VERTEX_SRC_PATH = "./shaders/sky.vs.glsl";
    const char* FRAGMENT_SRC_PATH = "./shaders/sky.fs.glsl";
    
    name_ = "Sky";
    loadShaderSource(GL_VERTEX_SHADER, VERTEX_SRC_PATH);
    loadShaderSource(GL_FRAGMENT_SHADER, FRAGMENT_SRC_PATH);
    link();
}

void Sky::getAllUniformLocations()
{
    // TODO: Initialiser les valeurs de uniform location en attributs
    mvpULoc = glGetUniformLocation(id_, "mvp");
}


void PhongShading::load()
{
    const char* VERTEX_SRC_PATH = "./shaders/phong.vs.glsl";
    const char* FRAGMENT_SRC_PATH = "./shaders/phong.fs.glsl";
    
    name_ = "PhongShading";
    loadShaderSource(GL_VERTEX_SHADER, VERTEX_SRC_PATH);
    loadShaderSource(GL_FRAGMENT_SHADER, FRAGMENT_SRC_PATH);
    link();
}

void PhongShading::getAllUniformLocations()
{
    mvpULoc = glGetUniformLocation(id_, "mvp");
    viewULoc = glGetUniformLocation(id_, "view");
    modelViewULoc = glGetUniformLocation(id_, "modelView");
    normalULoc = glGetUniformLocation(id_, "normalMatrix");
    
    nSpotLightsULoc = glGetUniformLocation(id_, "nSpotLights");
    
    globalAmbientULoc = glGetUniformLocation(id_, "globalAmbient");
}

void PhongShading::assignAllUniformBlockIndexes()
{
    setUniformBlockBinding("MaterialBlock", 0);
    setUniformBlockBinding("LightingBlock", 1);
}


void PhongShading::setMatrices(glm::mat4& mvp, glm::mat4& view, glm::mat4& model)
{
    glm::mat4 modelView = view * model;
    
    glUniformMatrix4fv(viewULoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(mvpULoc, 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix4fv(modelViewULoc, 1, GL_FALSE, &modelView[0][0]);
    glUniformMatrix3fv(normalULoc, 1, GL_TRUE, glm::value_ptr(glm::inverse(glm::mat3(modelView))));
}


void GrassShader::load()
{
    name_ = "Grass";
    loadShaderSource(GL_VERTEX_SHADER, "./shaders/grass.vs.glsl");
    loadShaderSource(GL_TESS_CONTROL_SHADER, "./shaders/grass.tcs.glsl");
    loadShaderSource(GL_TESS_EVALUATION_SHADER, "./shaders/grass.tes.glsl");
    loadShaderSource(GL_GEOMETRY_SHADER, "./shaders/grass.gs.glsl");
    loadShaderSource(GL_FRAGMENT_SHADER, "./shaders/grass.fs.glsl");
    link();
}

void GrassShader::getAllUniformLocations()
{
    mvpULoc = glGetUniformLocation(id_, "mvp");
    modelViewULoc = glGetUniformLocation(id_, "modelView");
}

void ParticleComputeShader::load()
{
    name_ = "ParticleCompute";
    loadShaderSource(GL_COMPUTE_SHADER, "./shaders/particlesUpdate.cs.glsl");
    link();
}

void ParticleComputeShader::getAllUniformLocations()
{
    deltaTimeULoc = glGetUniformLocation(id_, "deltaTime");
    emitterPositionULoc = glGetUniformLocation(id_, "emitterPosition");
    emitterDirectionULoc = glGetUniformLocation(id_, "emitterDirection");
    timeULoc = glGetUniformLocation(id_, "time");
    nParticlesULoc = glGetUniformLocation(id_, "nParticles");
    emitterActiveULoc = glGetUniformLocation(id_, "emitterActive");
}


void ParticleRenderShader::load()
{
    name_ = "ParticleRender";
    loadShaderSource(GL_VERTEX_SHADER, "./shaders/particlesDraw.vs.glsl");
    loadShaderSource(GL_GEOMETRY_SHADER, "./shaders/particlesDraw.gs.glsl");
    loadShaderSource(GL_FRAGMENT_SHADER, "./shaders/particlesDraw.fs.glsl");
    link();
}

void ParticleRenderShader::getAllUniformLocations()
{
    modelViewULoc = glGetUniformLocation(id_, "modelView");
    projectionULoc = glGetUniformLocation(id_, "projection");
    textureULoc = glGetUniformLocation(id_, "particleTexture");
}
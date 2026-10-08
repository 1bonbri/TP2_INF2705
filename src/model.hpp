#pragma once

#include <glbinding/gl/gl.h>

using namespace gl;

class Model
{
public:
    void load(const char* path);
    
    ~Model();
    
    void draw();
    void load(float* vertices, size_t verticesSize, unsigned int* elements, size_t elementsSize);

private:
    GLuint vao_, vbo_, ebo_;
    GLsizei count_;
};



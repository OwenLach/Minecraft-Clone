#pragma once

#include <memory>

#include "Shader.h"
#include "OpenGL/VertexArray.h"
#include "OpenGL/VertexBuffer.h"

class Crosshair
{
public:
    Crosshair();
    void render();

private:
    VertexArray vao;
    VertexBuffer vbo;

    std::unique_ptr<Shader> shader;

    void createShader();
    void configureVertexAttributes();
};
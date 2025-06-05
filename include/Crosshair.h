#pragma once

#include <memory>

#include "Shader.h"
#include "VertexArray.h"
#include "VertexBuffer.h"

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
#include <glad/glad.h>
#include <iostream>

#include "Crosshair.h"
#include "Constants.h"

Crosshair::Crosshair()
{

    createShader();
    configureVertexAttributes();
}

void Crosshair::render()
{
    glDisable(GL_DEPTH_TEST);
    shader->use();
    vao.bind();
    vbo.bind();
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 2, 2);
    vao.unbind();
    glEnable(GL_DEPTH_TEST);
}

void Crosshair::createShader()
{
    // Compile and link shader (implementation depends on your Shader class)
    shader = std::make_unique<Shader>("../shaders/crosshair.vert", "../shaders/crosshair.frag");
}

void Crosshair::configureVertexAttributes()
{
    const float crosshairPixels = 10.0f;

    const float horizontal = crosshairPixels / (Constants::SCREEN_W / 2);
    const float vertical = crosshairPixels / (Constants::SCREEN_H / 2);

    float vertices[] = {
        // Horizontal line
        -horizontal, 0.0f, // Left
        horizontal, 0.0f,  // Right
        // Vertical line
        0.0f, -vertical, // Bottom
        0.0f, vertical   // Top
    };

    vao.bind();
    vbo.setData(vertices, sizeof(vertices));

    VertexBufferLayout layout;
    layout.push<float>(2); // position
    vao.addBuffer(vbo, layout);

    vao.unbind();
    vbo.unbind();
}

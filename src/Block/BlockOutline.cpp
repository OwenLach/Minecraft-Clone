#include "Block/BlockOutline.h"
#include "Shader.h"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

BlockOutline::BlockOutline()
{
    createShader();
    configureVertexAttributes();
    std::cout << "Block Outline Shader and vertex attributes initialized" << std::endl;
}

void BlockOutline::render(const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &blockPosition)
{
    shader_->use();
    shader_->setMat4("model", glm::translate(glm::mat4(1.0f), blockPosition - glm::vec3(0.5f)));
    shader_->setMat4("view", view);
    shader_->setMat4("projection", projection);
    shader_->setVec3("outlineColor", glm::vec3(0.0f, 0.0f, 0.0f)); // Black outline

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(4.0f);

    vao_.bind();
    vbo_.bind();
    ebo_.bind();

    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

    // Restore normal rendering state
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth(1.0f);
}

void BlockOutline::createShader()
{
    shader_ = std::make_unique<Shader>("../shaders/block_outline.vert", "../shaders/block_outline.frag");
}

void BlockOutline::configureVertexAttributes()
{
    vao_.bind();
    vbo_.bind();
    ebo_.bind();

    float vertices[] = {
        0.0f, 0.0f, 0.0f, // bottom-left-back
        1.0f, 0.0f, 0.0f, //
        1.0f, 0.0f, 1.0f, //
        0.0f, 0.0f, 1.0f, //
                          //
        0.0f, 1.0f, 0.0f, //
        1.0f, 1.0f, 0.0f, //
        1.0f, 1.0f, 1.0f, //
        0.0f, 1.0f, 1.0f, //
    };

    // Wireframe indices (lines connecting cube edges)
    unsigned int indices[] = {
        // Bottom face edges
        0, 1, 1, 2, 2, 3, 3, 0,
        // Top face edges
        4, 5, 5, 6, 6, 7, 7, 4,
        // Vertical edges
        0, 4, 1, 5, 2, 6, 3, 7};

    vao_.bind();
    vbo_.setData(vertices, sizeof(vertices));
    ebo_.setData(indices, sizeof(indices));

    VertexBufferLayout layout;
    layout.push<float>(3); // position
    vao_.addBuffer(vbo_, layout);
}
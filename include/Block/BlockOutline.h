#pragma once
#include <memory>

#include "Shader.h"
#include "OpenGL/VertexArray.h"
#include "OpenGL/VertexBuffer.h"
#include "OpenGL/ElementBuffer.h"

class BlockOutline
{
public:
    BlockOutline();
    void render(const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &blockPosition);

private:
    VertexArray vao_;
    VertexBuffer vbo_;
    ElementBuffer ebo_;
    std::unique_ptr<Shader> shader_;

    void createShader();
    void configureVertexAttributes();
};
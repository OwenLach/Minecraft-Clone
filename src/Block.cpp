#include <glad/glad.h>
#include "Block.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Block::Block(BlockType type, TextureAtlas *atlas, glm::vec3 pos, Shader *shader)
    : block_type(type), textureAtlas(atlas), position(pos), shader(shader)
{
    initBlockTextures();
    initVerticies();
    setupMesh();
}

void Block::setupMesh()
{
    vao.bind();
    vboPtr = std::make_unique<VertexBuffer>(verticies.data(), verticies.size() * sizeof(float));
    vboPtr->bind();

    VertexBufferLayout layout;
    layout.push<float>(3); // position
    layout.push<float>(2); // texture coords
    vao.addBuffer(*vboPtr, layout);
}

void Block::render()
{
    if (!shader)
    {
        std::cerr << "No shader in Block.cpp" << std::endl;
        return;
    }

    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    shader->setMat4("model", model);

    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

std::vector<float> Block::getVerticies()
{
    return verticies;
}

void Block::initBlockTextures()
{
    if (!textureAtlas)
    {
        std::cerr << "ERROR: textureAtlas is null in Block::initBlockTextures()\n";
        return;
    }

    textureUVs = textureAtlas->getUVcoords(block_type);
}

void Block::initVerticies()
{
    verticies = {
        // Back face
        -0.5f, -0.5f, -0.5f, textureUVs.side.bottomLeft.x, textureUVs.side.bottomLeft.y,
        0.5f, -0.5f, -0.5f, textureUVs.side.bottomRight.x, textureUVs.side.bottomRight.y,
        0.5f, 0.5f, -0.5f, textureUVs.side.topRight.x, textureUVs.side.topRight.y,
        0.5f, 0.5f, -0.5f, textureUVs.side.topRight.x, textureUVs.side.topRight.y,
        -0.5f, 0.5f, -0.5f, textureUVs.side.topLeft.x, textureUVs.side.topLeft.y,
        -0.5f, -0.5f, -0.5f, textureUVs.side.bottomLeft.x, textureUVs.side.bottomLeft.y,

        // Front face
        -0.5f, -0.5f, 0.5f, textureUVs.side.bottomLeft.x, textureUVs.side.bottomLeft.y,
        0.5f, -0.5f, 0.5f, textureUVs.side.bottomRight.x, textureUVs.side.bottomRight.y,
        0.5f, 0.5f, 0.5f, textureUVs.side.topRight.x, textureUVs.side.topRight.y,
        0.5f, 0.5f, 0.5f, textureUVs.side.topRight.x, textureUVs.side.topRight.y,
        -0.5f, 0.5f, 0.5f, textureUVs.side.topLeft.x, textureUVs.side.topLeft.y,
        -0.5f, -0.5f, 0.5f, textureUVs.side.bottomLeft.x, textureUVs.side.bottomLeft.y,

        // Left face
        -0.5f, 0.5f, 0.5f, textureUVs.side.topRight.x, textureUVs.side.topRight.y,
        -0.5f, 0.5f, -0.5f, textureUVs.side.topLeft.x, textureUVs.side.topLeft.y,
        -0.5f, -0.5f, -0.5f, textureUVs.side.bottomLeft.x, textureUVs.side.bottomLeft.y,
        -0.5f, -0.5f, -0.5f, textureUVs.side.bottomLeft.x, textureUVs.side.bottomLeft.y,
        -0.5f, -0.5f, 0.5f, textureUVs.side.bottomRight.x, textureUVs.side.bottomRight.y,
        -0.5f, 0.5f, 0.5f, textureUVs.side.topRight.x, textureUVs.side.topRight.y,

        // Right face
        0.5f, 0.5f, 0.5f, textureUVs.side.topRight.x, textureUVs.side.topRight.y,
        0.5f, 0.5f, -0.5f, textureUVs.side.topLeft.x, textureUVs.side.topLeft.y,
        0.5f, -0.5f, -0.5f, textureUVs.side.bottomLeft.x, textureUVs.side.bottomLeft.y,
        0.5f, -0.5f, -0.5f, textureUVs.side.bottomLeft.x, textureUVs.side.bottomLeft.y,
        0.5f, -0.5f, 0.5f, textureUVs.side.bottomRight.x, textureUVs.side.bottomRight.y,
        0.5f, 0.5f, 0.5f, textureUVs.side.topRight.x, textureUVs.side.topRight.y,

        // Bottom face
        -0.5f, -0.5f, -0.5f, textureUVs.bottom.bottomLeft.x, textureUVs.bottom.bottomLeft.y,
        0.5f, -0.5f, -0.5f, textureUVs.bottom.bottomRight.x, textureUVs.bottom.bottomRight.y,
        0.5f, -0.5f, 0.5f, textureUVs.bottom.topRight.x, textureUVs.bottom.topRight.y,
        0.5f, -0.5f, 0.5f, textureUVs.bottom.topRight.x, textureUVs.bottom.topRight.y,
        -0.5f, -0.5f, 0.5f, textureUVs.bottom.topLeft.x, textureUVs.bottom.topLeft.y,
        -0.5f, -0.5f, -0.5f, textureUVs.bottom.bottomLeft.x, textureUVs.bottom.bottomLeft.y,

        // Top face
        -0.5f, 0.5f, -0.5f, textureUVs.top.bottomLeft.x, textureUVs.top.bottomLeft.y,
        0.5f, 0.5f, -0.5f, textureUVs.top.bottomRight.x, textureUVs.top.bottomRight.y,
        0.5f, 0.5f, 0.5f, textureUVs.top.topRight.x, textureUVs.top.topRight.y,
        0.5f, 0.5f, 0.5f, textureUVs.top.topRight.x, textureUVs.top.topRight.y,
        -0.5f, 0.5f, 0.5f, textureUVs.top.topLeft.x, textureUVs.top.topLeft.y,
        -0.5f, 0.5f, -0.5f, textureUVs.top.bottomLeft.x, textureUVs.top.bottomLeft.y};
}

void Block::setPosition(glm::vec3 pos) { position = pos; }

glm::vec3 Block::getPosition() { return position; }

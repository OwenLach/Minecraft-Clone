#include "Block.h"

Block::Block(BlockType type, TextureAtlas *atlas, glm::vec3 pos)
    : block_type(type), textureAtlas(atlas), position(pos)
{
    initBlockTextures();
}

std::vector<float> Block::getVerticies()
{
    return {
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

void Block::initBlockTextures()
{
    textureUVs = textureAtlas->getUVcoords(block_type);
}

void Block::setPosition(glm::vec3 pos) { position = pos; }

glm::vec3 Block::getPosition() { return position; }

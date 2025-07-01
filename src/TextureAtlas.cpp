#include "TextureAtlas.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <iostream>

TextureAtlas::TextureAtlas()
    : tileSize(16), ID(0), atlasWidth(0), atlasHeight(0)
{
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // load image, create texture and generate mipmaps
    int nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *data = stbi_load("../textures/texture-atlas.png", &atlasWidth, &atlasHeight, &nrChannels, 0);
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, atlasWidth, atlasHeight, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    initIndicies();
}

std::vector<glm::vec2> TextureAtlas::getFaceUVs(BlockType type, BlockFaces face)
{
    std::vector<glm::vec2> faceUVs;

    if (face == BlockFaces::Top)
    {
        faceUVs = getTileUVs(blockIndicies[type].top.x, blockIndicies[type].top.y);
    }
    else if (face == BlockFaces::Left || face == BlockFaces::Right || face == BlockFaces::Front || face == BlockFaces::Back)
    {
        faceUVs = getTileUVs(blockIndicies[type].side.x, blockIndicies[type].side.y);
    }
    else if (face == BlockFaces::Bottom)
    {
        faceUVs = getTileUVs(blockIndicies[type].bottom.x, blockIndicies[type].bottom.y);
    }

    return faceUVs;
}

void TextureAtlas::initIndicies()
{
    blockIndicies[BlockType::Grass] = {glm::ivec2(0, 0),  // top
                                       glm::ivec2(3, 0),  // side
                                       glm::ivec2(2, 0)}; // bottom

    blockIndicies[BlockType::Stone] = {glm::ivec2(1, 0),  // top
                                       glm::ivec2(1, 0),  // side
                                       glm::ivec2(1, 0)}; // bottom

    blockIndicies[BlockType::Cobblestone] = {glm::ivec2(0, 1),  // top
                                             glm::ivec2(0, 1),  // side
                                             glm::ivec2(0, 1)}; // bottom

    blockIndicies[BlockType::CryingObsidian] = {glm::ivec2(5, 2),  // top
                                                glm::ivec2(5, 2),  // side
                                                glm::ivec2(5, 2)}; // bottom

    blockIndicies[BlockType::Dirt] = {glm::ivec2(2, 0),  // top
                                      glm::ivec2(2, 0),  // side
                                      glm::ivec2(2, 0)}; // bottom
}

std::vector<glm::vec2> TextureAtlas::getTileUVs(int tileX, int tileY)
{
    int numTilesX = atlasWidth / tileSize;  // 16
    int numTilesY = atlasHeight / tileSize; // 16

    // Flip tileY because UV origin is bottom-left but tile indexing may be top-left
    int flippedTileY = numTilesY - 1 - tileY;

    glm::vec2 uvOffset = glm::vec2(
        tileX / (float)numTilesX,
        flippedTileY / (float)numTilesY);

    glm::vec2 tileUVsize = glm::vec2(
        1.0f / (float)numTilesX,
        1.0f / (float)numTilesY);

    glm::vec2 topLeft = uvOffset + glm::vec2(0.0f, tileUVsize.y);
    glm::vec2 topRight = uvOffset + glm::vec2(tileUVsize.x, tileUVsize.y);
    glm::vec2 bottomLeft = uvOffset;
    glm::vec2 bottomRight = uvOffset + glm::vec2(tileUVsize.x, 0.0f);

    return {
        bottomLeft, bottomRight, topRight,
        topRight, topLeft, bottomLeft};
}

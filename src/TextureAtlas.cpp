#include "TextureAtlas.h"
#include "Block/BlockFaceData.h"

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

    initBlockUVs();
}

void TextureAtlas::initBlockUVs()
{
    std::unordered_map<BlockType, BlockTextureAtlasIndicies> blockIndicies;
    //                                        top             side                bottom
    blockIndicies[BlockType::Grass] = {glm::ivec2(0, 0), glm::ivec2(3, 0), glm::ivec2(2, 0)};
    blockIndicies[BlockType::Stone] = {glm::ivec2(1, 0), glm::ivec2(1, 0), glm::ivec2(1, 0)};
    blockIndicies[BlockType::Cobblestone] = {glm::ivec2(0, 1), glm::ivec2(0, 1), glm::ivec2(0, 1)};
    blockIndicies[BlockType::Dirt] = {glm::ivec2(2, 0), glm::ivec2(2, 0), glm::ivec2(2, 0)};

    for (const auto &[type, indices] : blockIndicies)
    {
        BlockUVs blockUVs;

        blockUVs.top = getTileUVs(indices.top.x, indices.top.y);
        blockUVs.side = getTileUVs(indices.side.x, indices.side.y);
        blockUVs.bottom = getTileUVs(indices.bottom.x, indices.bottom.y);

        blockUVsMap[type] = blockUVs;
    }
}

const std::array<glm::vec2, 4> &TextureAtlas::getBlockFaceUVs(BlockType type, BlockFaces face)
{
    const auto &blockUVs = blockUVsMap.at(type);

    if (face == BlockFaces::Top)
    {
        return blockUVs.top;
    }
    else if (face == BlockFaces::Bottom)
    {
        return blockUVs.bottom;
    }
    else
    {
        return blockUVs.side;
    }
}

std::array<glm::vec2, 4> TextureAtlas::getTileUVs(int tileX, int tileY)
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

    return {bottomLeft, bottomRight, topRight, topLeft};

    // return {
    //     bottomLeft, bottomRight, topRight,
    //     topRight, topLeft, bottomLeft};
}

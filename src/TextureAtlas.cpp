#include "TextureAtlas.h"

#include <glad/glad.h>
#include <stb_image.h>

#include <iostream>

TextureAtlas::TextureAtlas()
    : tileSize(16)
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

BlockTextureUVs TextureAtlas::getUVcoords(BlockType type)
{
    BlockTextureAtlasIndicies tile_coords = blockIndicies[type];

    UVCoords topUVs = getTileUVs(tile_coords.top.x, tile_coords.top.y);
    UVCoords sideUVs = getTileUVs(tile_coords.side.x, tile_coords.side.y);
    UVCoords bottomUVs = getTileUVs(tile_coords.bottom.x, tile_coords.bottom.y);

    return BlockTextureUVs{topUVs, sideUVs, bottomUVs};
}

void TextureAtlas::initIndicies()
{
    blockIndicies[BlockType::Grass] = {glm::vec2(0, 0),  // top
                                       glm::vec2(3, 0),  // side
                                       glm::vec2(2, 0)}; // bottom

    blockIndicies[BlockType::Stone] = {glm::vec2(1, 0),  // top
                                       glm::vec2(1, 0),  // side
                                       glm::vec2(1, 0)}; // bottom

    blockIndicies[BlockType::Cobblestone] = {glm::vec2(0, 1),  // top
                                             glm::vec2(0, 1),  // side
                                             glm::vec2(0, 1)}; // bottom

    blockIndicies[BlockType::CryingObsidian] = {glm::vec2(5, 2),  // top
                                                glm::vec2(5, 2),  // side
                                                glm::vec2(5, 2)}; // bottom
}

UVCoords TextureAtlas::getTileUVs(int tileX, int tileY)
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

    return UVCoords{topLeft, topRight, bottomLeft, bottomRight};
}

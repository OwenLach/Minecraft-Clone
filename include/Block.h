#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include "Constants.h"
#include "BlockTypes.h"
#include "TextureAtlas.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "Shader.h"

class Block
{
public:
    BlockType block_type;
    TextureAtlas *textureAtlas;
    BlockTextureUVs textureUVs;

    /**
     * @brief Constructs a Block with the specified type, texture atlas, and position.
     *
     * @param type The type of the block (e.g., Grass, Stone, Air).
     * @param atlas Pointer to the TextureAtlas used for rendering this block's textures.
     * @param pos The world-space position of the block as a 3D vector.
     *
     * This constructor initializes a block with all necessary rendering data, including
     * its visual representation (via the texture atlas) and its location in the world.
     */
    Block(BlockType type, TextureAtlas *atlas, glm::vec3 pos, Shader *shader);
    void setupMesh();
    void render();
    std::vector<float> getVerticies();

private:
    VertexArray vao;
    VertexBuffer vbo;

    Shader *shader;

    std::vector<float> verticies;
    glm::vec3 position;

    void initBlockTextures();
    void initVerticies();
    void setPosition(glm::vec3 pos);
    glm::vec3 getPosition();
};
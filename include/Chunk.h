#pragma once

#include <vector>
#include <memory>

#include "Block.h"
#include "Constants.h"
#include "TextureAtlas.h"
#include "Shader.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 uvs;
};

class Chunk
{
public:
    Chunk(Shader &shader, TextureAtlas *atlas, glm::vec3 pos);
    void renderChunk();
    bool blockInBounds(glm::ivec3 pos) const;

    /**
     * @brief Returns a reference to the block at the given position.
     *
     * Returns a default air block if the position is outside the chunk bounds.
     *
     * @param pos The block position within the chunk.
     * @return Block& Reference to the block or a default air block if out of bounds.
     */
    const Block &getBlockAt(glm::ivec3 pos) const;

    /**
     * @brief Orchestrates the process of updating the chunk's visual data on the GPU.
     * It calls generateChunkMesh() to get the latest vertex data and then uploads
     * it to the vertex buffer object (VBO).
     */
    void rebuildMesh();

private:
    VertexArray vao;
    VertexBuffer vbo;
    Shader &shader;
    TextureAtlas *textureAtlas;
    std::vector<Vertex> vertices;
    std::vector<Block> blocks;
    glm::vec3 worldPos;
    /**
     * @brief Sets up the OpenGL vertex array object (VAO) and defines the layout
     * of the vertex data for the chunk. This function is typically called
     * once during the initialization of the Chunk.
     */
    void setupChunkMesh();

    /**
     * @brief Generates the raw vertex data (positions, texture coordinates, etc.)
     * that represents the visual geometry of the chunk. This function
     * iterates through the blocks and creates vertices for the visible faces.
     * @return std::vector<Vertex> A vector containing the vertex data for the chunk.
     */
    std::vector<Vertex> generateChunkMesh();

    /**
     * @brief Generates the visible mesh data for a single block.
     *
     * This function checks each face of the given block to determine if it should
     * be rendered (i.e., the adjacent block is air). If a face is visible, it
     * generates the corresponding vertex data and appends it to the provided
     * vertex buffer.
     *
     * @param block The block for which mesh data is to be generated.
     * @param meshVerts A reference to the vector where the generated vertex data
     *        will be appended. Each face contributes 6 vertices (2 triangles).
     */
    void generateBlockMesh(const Block &block, std::vector<Vertex> &meshVerts);
    BlockType getBlockTypeAt(glm::ivec3 pos) const;
    glm::ivec3 getNeighborPosition(glm::ivec3 pos, BlockFaces face) const;
};
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
#include "ChunkCoord.h"

class World;

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 uvs;
};

class Chunk
{
public:
    Chunk(Shader &shader, TextureAtlas *atlas, ChunkCoord pos, World *world);

    void renderChunk();
    void updateMesh();
    void configureVertexAttributes();
    void setDirty();

    const Block &getBlockAt(const glm::ivec3 &pos) const;
    void setBlockAt(glm::ivec3 pos, Block &block);

    inline bool blockInChunkBounds(const glm::ivec3 &pos) const;

private:
    std::vector<Vertex> vertices;
    std::vector<Block> blocks;
    size_t vertexCount = 0;

    Shader &shader;
    TextureAtlas *textureAtlas;
    VertexArray vao;
    VertexBuffer vbo;

    ChunkCoord worldPos;
    World *world;

    bool isDirty;
    bool modelMatrixDirty = true;

    glm::mat4 modelMatrix;

    void generateMesh();
    void generateTerrain();
    void generateBlockMesh(const Block &block);
    void addBlockFace(const Block &block, const BlockType type, const BlockFaces face);

    BlockType getNeighborBlockType(const glm::ivec3 blockPos, const glm::ivec3 offset);

    inline bool isTransparent(BlockType type) const;
    inline size_t getBlockIndex(const glm::ivec3 &pos) const;
};
#pragma once

#include "Block/Block.h"
#include "Constants.h"
#include "TextureAtlas.h"
#include "Shader.h"
#include "Chunk/ChunkCoord.h"
#include "Chunk/ChunkStateMachine.h"
#include "Chunk/ChunkMesh.h"
#include "Chunk/Vertex.h"
#include "FastNoiseLite.h"
#include "TerrainGenerator.h"
#include "OpenGL/VertexArray.h"
#include "OpenGL/VertexBuffer.h"
#include "OpenGL/ElementBuffer.h"
#include "OpenGL/VertexBufferLayout.h"

#include <vector>
#include <memory>
#include <atomic>
#include <functional>

#include <glm/glm.hpp>

class ChunkManager;
class ChunkPipeline;
class MeshData;

struct BoundingBox
{
    glm::vec3 min;
    glm::vec3 max;
};

class Chunk : public std::enable_shared_from_this<Chunk>
{

public:
    Chunk(Shader &shader, TextureAtlas &atlas, ChunkCoord pos);

    // Core methods
    void render();
    void generateTerrain();

    // Operate on own block data
    void removeBlockAt(glm::ivec3 pos);
    void setBlockAt(glm::ivec3 pos, BlockType type);

    // State
    ChunkState getState() const;
    void setState(ChunkState newState);
    bool canUnload() const;
    bool isProcessing();
    bool canRemesh();

    // Getters/Setters
    ChunkMesh &getMesh();
    void setMeshData(MeshData &newMeshData);
    const ChunkCoord getCoord() const;
    const BoundingBox getBoundingBox() const;
    const TextureAtlas &getTextureAtlasRef() const;
    Block &getBlockLocal(const glm::ivec3 &pos);

    inline bool blockPosInChunkBounds(const glm::ivec3 &pos) const
    {
        return pos.x >= 0 && pos.x < Constants::CHUNK_SIZE_X &&
               pos.y >= 0 && pos.y < Constants::CHUNK_SIZE_Y &&
               pos.z >= 0 && pos.z < Constants::CHUNK_SIZE_Z;
    }

private:
    // ---- Core Data ------
    std::vector<Block> blocks_;
    ChunkMesh mesh_;
    ChunkStateMachine stateMachine_;
    ChunkCoord chunkCoord_;
    BoundingBox boundingBox_;
    TerrainGenerator terrainGen_;
    TextureAtlas &textureAtlas_;
    glm::mat4 modelMatrix_;

    inline size_t getBlockIndex(const glm::ivec3 &pos) const;
};
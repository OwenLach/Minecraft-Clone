#pragma once

#include "Block/BlockFaceData.h"

#include <vector>
#include <memory>
#include <functional>
#include <array>

class Block;
class Chunk;
class MeshData;
class TextureAtlas;

// Responsible for generating a mesh (vertices and indices) for a chunk
class ChunkMeshBuilder
{
public:
    ChunkMeshBuilder(MeshData &meshData, const TextureAtlas &atlas, std::shared_ptr<Chunk> chunk, const std::array<std::shared_ptr<Chunk>, 4> &neighborChunks);
    MeshData &buildMesh();

private:
    MeshData &meshData_;
    std::shared_ptr<Chunk> chunk_;
    const std::array<std::shared_ptr<Chunk>, 4> &neighborChunks_;
    const TextureAtlas &textureAtlas_;

    void generateBlockMesh(Block &block);
    void generateFaceMesh(Block &block, BlockFaces face, const std::function<BlockType(int, int, int)> &getCachedNeighbor);

    BlockType getNeighborBlockType(const glm::ivec3 blockPos, const glm::ivec3 offset);
    bool isBlockHiddenByNeighbors(const glm::ivec3 &pos);
    inline bool isTransparent(BlockType type) const;
};
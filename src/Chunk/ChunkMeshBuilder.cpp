#include "Chunk/ChunkMeshBuilder.h"
#include "Chunk/Chunk.h"
#include "Chunk/MeshData.h"
#include "Block/BlockFaceData.h"
#include "Constants.h"
#include "TextureAtlas.h"

#include <glm/glm.hpp>
#include <iostream>

ChunkMeshBuilder::ChunkMeshBuilder(MeshData &meshData, const TextureAtlas &atlas, std::shared_ptr<Chunk> chunk, const std::array<std::shared_ptr<Chunk>, 4> &neighborChunks)
    : meshData_(meshData), textureAtlas_(atlas), chunk_(chunk), neighborChunks_(neighborChunks)
{
}

MeshData &ChunkMeshBuilder::buildMesh()
{
    using namespace Constants;
    // loop through chunk and generate each blocks mesh

    for (int x = 0; x < CHUNK_SIZE_X; x++)
    {
        for (int y = 0; y < CHUNK_SIZE_Y; y++)
        {
            for (int z = 0; z < CHUNK_SIZE_Z; z++)
            {
                glm::ivec3 pos = glm::ivec3(x, y, z);
                Block &block = chunk_->getBlockLocal(pos);

                if (block.type == BlockType::Air)
                    continue;
                if (isBlockHiddenByNeighbors(pos))
                    continue;

                generateBlockMesh(block);
            }
        }
    }

    return meshData_;
}

void ChunkMeshBuilder::generateBlockMesh(Block &block)
{
    // ================NEIGHBOR CACHING===================================
    BlockType cache[27];
    for (int i = 0; i < 27; i++)
    {
        glm::ivec3 offset = glm::ivec3(i % 3 - 1, (i / 3) % 3 - 1, i / 9 - 1);
        cache[i] = getNeighborBlockType(block.position, offset);
    }

    // Helper to get neighbor from cache
    auto getNeighborTypeFromCache = [&](int x, int y, int z)
    {
        // Convert offset from [-1, 1] to [0, 2] for array indexing
        return cache[(x + 1) + (y + 1) * 3 + (z + 1) * 9];
    };
    // ========================================================================

    for (int f = 0; f < 6; f++)
    {
        const auto offset = BlockFaceData::FACE_OFFSETS[f];
        if (isTransparent(getNeighborTypeFromCache(offset.x, offset.y, offset.z)))
            generateFaceMesh(block, static_cast<BlockFaces>(f), getNeighborTypeFromCache);
    }
}

void ChunkMeshBuilder::generateFaceMesh(Block &block, BlockFaces face, const std::function<BlockType(int, int, int)> &getNeighborTypeFromCache)
{
    // std::cout << "Light value for block: " << static_cast<float>(block.skylight) / 15.0f << std::endl;
    const auto &faceUVs = textureAtlas_.getBlockFaceUVs(block.type, face);
    const auto &corners = BlockFaceData::faceCorners.at(face);
    const auto &aoData = BlockFaceData::aoOffsets.at(face);

    // ao helper function
    auto computeAO = [&](const std::array<glm::ivec3, 3> &offsets)
    {
        bool side1 = !isTransparent(getNeighborTypeFromCache(offsets[0].x, offsets[0].y, offsets[0].z));
        bool side2 = !isTransparent(getNeighborTypeFromCache(offsets[1].x, offsets[1].y, offsets[1].z));
        bool corner = !isTransparent(getNeighborTypeFromCache(offsets[2].x, offsets[2].y, offsets[2].z));

        if (side1 && side2)
        {
            return 0.3f; // Darkest
        }
        int occlusion = side1 + side2 + corner;
        return 1.0f - occlusion * 0.2f; // Simple mapping: 1.0, 0.8, 0.6, 0.4
    };

    unsigned int baseVertexIndex = static_cast<unsigned int>(meshData_.vertices_.size());
    // Make vertex for each corner of face
    for (int i = 0; i < 4; i++)
    {
        Vertex v;
        v.position = corners[i] + glm::vec3(block.position);
        v.textureCoords = faceUVs[i];
        v.ao = computeAO(aoData[i]);
        // // Skylight = 0-15, so normalize to [0 - 1] for OpenGL
        // v.light = static_cast<float>(block.skylight) / 15.0f;
        meshData_.vertices_.push_back(v);
    }

    for (size_t i = 0; i < 6; i++)
    {
        meshData_.indices_.push_back(baseVertexIndex + BlockFaceData::quadIndices[i]);
    }
}

BlockType ChunkMeshBuilder::getNeighborBlockType(const glm::ivec3 blockPos, const glm::ivec3 offset)
{
    const auto northChunk = neighborChunks_[0];
    const auto southChunk = neighborChunks_[1];
    const auto eastChunk = neighborChunks_[2];
    const auto westChunk = neighborChunks_[3];

    const glm::ivec3 neighborLocalPos = blockPos + offset;

    // if it's in the chunk, just get it
    if (chunk_->blockPosInChunkBounds(neighborLocalPos))
    {
        return chunk_->getBlockLocal(neighborLocalPos).type;
    }
    else
    {
        // Check which chunk block is in
        // Determine which neighboring chunk the block falls into and get its local coordinates

        if (neighborLocalPos.x < 0 &&
            neighborLocalPos.z >= 0 && neighborLocalPos.z < Constants::CHUNK_SIZE_Z &&
            neighborLocalPos.y >= 0 && neighborLocalPos.y < Constants::CHUNK_SIZE_Y)
        {
            if (westChunk)
            {
                glm::ivec3 localPosInNeighbor = {neighborLocalPos.x + Constants::CHUNK_SIZE_X, neighborLocalPos.y, neighborLocalPos.z};
                return westChunk->getBlockLocal(localPosInNeighbor).type;
            }
        }
        // Check East neighbor (+X direction)
        else if (neighborLocalPos.x >= Constants::CHUNK_SIZE_X &&
                 neighborLocalPos.z >= 0 && neighborLocalPos.z < Constants::CHUNK_SIZE_Z &&
                 neighborLocalPos.y >= 0 && neighborLocalPos.y < Constants::CHUNK_SIZE_Y)
        {
            if (eastChunk)
            {
                glm::ivec3 localPosInNeighbor = {neighborLocalPos.x - Constants::CHUNK_SIZE_X, neighborLocalPos.y, neighborLocalPos.z};
                return eastChunk->getBlockLocal(localPosInNeighbor).type;
            }
        }
        // Check South neighbor (-Z direction)
        else if (neighborLocalPos.z < 0 &&
                 neighborLocalPos.x >= 0 && neighborLocalPos.x < Constants::CHUNK_SIZE_X &&
                 neighborLocalPos.y >= 0 && neighborLocalPos.y < Constants::CHUNK_SIZE_Y)
        {
            if (southChunk)
            {
                glm::ivec3 localPosInNeighbor = {neighborLocalPos.x, neighborLocalPos.y, neighborLocalPos.z + Constants::CHUNK_SIZE_Z};
                return southChunk->getBlockLocal(localPosInNeighbor).type;
            }
        }
        // Check North neighbor (+Z direction)
        else if (neighborLocalPos.z >= Constants::CHUNK_SIZE_Z &&
                 neighborLocalPos.x >= 0 && neighborLocalPos.x < Constants::CHUNK_SIZE_X &&
                 neighborLocalPos.y >= 0 && neighborLocalPos.y < Constants::CHUNK_SIZE_Y)
        {
            if (northChunk)
            {
                glm::ivec3 localPosInNeighbor = {neighborLocalPos.x, neighborLocalPos.y, neighborLocalPos.z - Constants::CHUNK_SIZE_Z};
                return northChunk->getBlockLocal(localPosInNeighbor).type;
            }
        }

        // Fallback
        return BlockType::Air;
    }
}

bool ChunkMeshBuilder::isBlockHiddenByNeighbors(const glm::ivec3 &pos)
{
    // Quick check if all 6 neighbors are solid blocks
    for (const auto &offset : BlockFaceData::FACE_OFFSETS)
    {
        glm::ivec3 neighborPos = pos + offset;
        if (chunk_->blockPosInChunkBounds(neighborPos))
        {
            if (isTransparent(chunk_->getBlockLocal(neighborPos).type))
            {
                return false;
            }
        }
        else
        {
            return false; // Edge blocks are never completely hidden
        }
    }
    return true;
}

inline bool ChunkMeshBuilder::isTransparent(BlockType type) const
{
    return type == BlockType::Air;
}
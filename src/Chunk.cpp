#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <array>

#include "Chunk.h"
#include "BlockTypes.h"
#include "World.h"
#include "FastNoiseLite.h"

static constexpr std::array<glm::ivec3, 6> FACE_OFFSETS = {{
    {1, 0, 0},  // Right
    {-1, 0, 0}, // Left
    {0, 1, 0},  // Top
    {0, -1, 0}, // Bottom
    {0, 0, 1},  // Front
    {0, 0, -1}  // Back
}};

Chunk::Chunk(Shader &shader, TextureAtlas *atlas, ChunkCoord pos, World *world)
    : shader(shader), textureAtlas(atlas), worldPos(pos), world(world)
{
    // reserve for worse case
    vertices.reserve(Constants::CHUNK_SIZE_X * Constants::CHUNK_SIZE_Y * Constants::CHUNK_SIZE_Z * 36);
    blocks.resize(Constants::CHUNK_SIZE_X * Constants::CHUNK_SIZE_Y * Constants::CHUNK_SIZE_Z);
    generateTerrain();
}

void Chunk::renderChunk()
{
    shader.use();
    vao.bind();

    if (modelMatrixDirty)
    {
        glm::mat4 model = glm::mat4(1.0f);
        modelMatrix = glm::translate(model, glm::vec3(worldPos.x * Constants::CHUNK_SIZE_X,
                                                      -Constants::CHUNK_SIZE_Y,
                                                      worldPos.z * Constants::CHUNK_SIZE_Z));
        modelMatrixDirty = false;
    }

    shader.setMat4("model", modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

void Chunk::updateMesh()
{
    vertices.clear();
    std::vector<float> vertexData;
    vertexData.reserve(vertices.capacity() * 5); // 5 floats per vertex

    generateMesh();

    // convert member var std::vector<Vertex> verticies to flat array
    for (const auto &vertex : vertices)
    {
        vertexData.insert(vertexData.end(), {vertex.pos.x, vertex.pos.y, vertex.pos.z,
                                             vertex.uvs.x, vertex.uvs.y});
    }
    vbo.setData(vertexData.data(), vertexData.size() * sizeof(float));
    vertexCount = vertexData.size();

    configureVertexAttributes();
}

void Chunk::configureVertexAttributes()
{
    vao.bind();
    VertexBufferLayout layout;
    layout.push<float>(3); // position
    layout.push<float>(2); // texture coords
    vao.addBuffer(vbo, layout);
}

void Chunk::setDirty()
{
    isDirty = true;
    modelMatrixDirty = true;
}

void Chunk::setBlockAt(glm::ivec3 pos, Block &block)
{
}

inline bool Chunk::blockInChunkBounds(const glm::ivec3 &pos) const
{
    return pos.x >= 0 && pos.x < Constants::CHUNK_SIZE_X &&
           pos.y >= 0 && pos.y < Constants::CHUNK_SIZE_Y &&
           pos.z >= 0 && pos.z < Constants::CHUNK_SIZE_Z;
}

const Block &Chunk::getBlockAt(const glm::ivec3 &pos) const
{
    if (!blockInChunkBounds(pos))
    {
        // Out of bounds — return a static air block
        static Block airBlock(BlockType::Air, glm::vec3(0));
        return airBlock;
    }

    const size_t index = getBlockIndex(pos);
    return blocks[index];
}

inline bool Chunk::isTransparent(BlockType type) const
{
    return type == BlockType::Air;
}

inline size_t Chunk::getBlockIndex(const glm::ivec3 &pos) const
{
    return pos.x + (pos.y * Constants::CHUNK_SIZE_X) + (pos.z * Constants::CHUNK_SIZE_X * Constants::CHUNK_SIZE_Y);
}

void Chunk::generateMesh()
{
    // loop through chunk and generate each blocks mesh
    for (int x = 0; x < Constants::CHUNK_SIZE_X; x++)
    {
        for (int y = 0; y < Constants::CHUNK_SIZE_Y; y++)
        {
            for (int z = 0; z < Constants::CHUNK_SIZE_Z; z++)
            {
                const Block &block = getBlockAt(glm::ivec3(x, y, z));

                if (block.type != BlockType::Air)
                    generateBlockMesh(block);
            }
        }
    }
}

void Chunk::generateTerrain()
{
    FastNoiseLite noise;
    noise.SetFrequency(0.01f);
    noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);

    for (int x = 0; x < Constants::CHUNK_SIZE_X; x++)
    {
        for (int z = 0; z < Constants::CHUNK_SIZE_Z; z++)
        {
            for (int y = 0; y < Constants::CHUNK_SIZE_Y; y++)
            {

                float worldX = worldPos.x * Constants::CHUNK_SIZE_X + x;
                float worldZ = worldPos.z * Constants::CHUNK_SIZE_Z + z;

                float noiseVal = noise.GetNoise(worldX, worldZ);
                int height = Constants::TERRAIN_BASE_HEIGHT + (int)(noiseVal * Constants::TERRAIN_HEIGHT_VARIATION);

                BlockType type;
                if (y < height - 5)
                {
                    type = BlockType::Stone;
                }
                else if (y < height)
                {
                    type = BlockType::Dirt;
                }
                else if (y == height)
                {
                    type = BlockType::Grass;
                }
                else
                {
                    type = BlockType::Air;
                }

                glm::ivec3 pos = {x, y, z};
                const size_t index = getBlockIndex(pos);
                blocks[index] = Block(type, pos);
            }
        }
    }
}

void Chunk::generateBlockMesh(const Block &block)
{

    if (block.type == BlockType::Air)
        return;

    // render faces if face doesn't have anything in front of it
    for (int faceIdx = 0; faceIdx < 6; faceIdx++)
    {
        const BlockFaces face = static_cast<BlockFaces>(faceIdx);
        const glm::ivec3 offset = FACE_OFFSETS[faceIdx];

        BlockType neighborType = getNeighborBlockType(block.position, offset);

        if (isTransparent(neighborType))
        {
            addBlockFace(block, block.type, face);
        }
    }
}

void Chunk::addBlockFace(const Block &block, const BlockType type, const BlockFaces face)
{
    const std::vector<glm::vec2> faceUVs = textureAtlas->getFaceUVs(type, face);
    const std::vector<float> rawVertices = block.generateFacevertices(face, faceUVs);

    // Reserve space to avoid reallocation
    vertices.reserve(vertices.size() + rawVertices.size() / 5);

    for (size_t i = 0; i < rawVertices.size(); i += 5)
    {
        vertices.push_back({
            {rawVertices[i], rawVertices[i + 1], rawVertices[i + 2]}, // pos
            {rawVertices[i + 3], rawVertices[i + 4]}                  // uvs
        });
    }
}

BlockType Chunk::getNeighborBlockType(const glm::ivec3 blockPos, const glm::ivec3 offset)
{
    const glm::ivec3 neighborLocalPos = blockPos + offset;

    // if it's in the chunk, just get it
    if (blockInChunkBounds(neighborLocalPos))
    {
        return getBlockAt(neighborLocalPos).type;
    }
    // get it using world coords
    else
    {
        // get blocks world positoins
        glm::ivec3 blockWorldPos = world->chunkToWorldCoords(worldPos, blockPos);
        // get neighbors world pos using blocks position
        glm::ivec3 neighborWorldPos = blockWorldPos + offset;
        // get type of neighbor
        return world->getBlockAt(neighborWorldPos).type;
    }
}
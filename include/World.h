#pragma once

#include <unordered_map>
#include <glm/glm.hpp>
#include "Chunk.h"
#include "Shader.h"
#include "TextureAtlas.h"
#include "ChunkCoord.h"
#include "Block.h"

class World
{
public:
    /**
     * @brief Constructs the world and initializes rendering systems.
     *
     * @param shader Reference to the main shader used for rendering.
     * @param atlas Pointer to the texture atlas used for block textures.
     */
    World(Shader &shader, TextureAtlas *atlas);

    /**
     * @brief Renders all currently loaded and visible chunks.
     *
     * Binds appropriate shaders and textures, uploads vertex data to the GPU,
     * and issues draw calls for chunk meshes.
     */
    void render();

    /**
     * @brief Updates world state, such as loading/unloading chunks and rebuilding meshes.
     *
     * Should be called every frame or tick to handle:
     * - Dirty chunk mesh regeneration
     * - Chunk loading and unloading based on player position
     * - Block and lighting updates
     */
    void update() const;

    /**
     * @brief Converts local block coordinates within a chunk to world coordinates.
     *
     * @param chunkCoords The chunk's grid position in the world (X and Z).
     * @param localPos The local position inside the chunk (X, Y, Z).
     * @return The corresponding world-space block coordinates.
     */
    glm::ivec3 chunkToWorldCoords(ChunkCoord chunkCoords, glm::ivec3 localPos) const;

    /**
     * @brief Converts a world-space block coordinate to its corresponding chunk coordinate.
     *
     * @param worldCoords The block's world coordinates.
     * @return A 3D chunk coordinate where the block resides (Y may be used for 3D chunk grids).
     */
    glm::ivec3 worldToChunkCoords(glm::ivec3 worldCoords) const;

    /**
     * @brief Retrieves a block from a specific chunk using chunk coordinates and local block position.
     *
     * @param chunkCoords The chunk grid position (X, Z).
     * @param blockPos The local position of the block within the chunk (X, Y, Z).
     * @return The requested Block, or an Air block if the chunk or position is invalid.
     */
    Block getBlockAt(ChunkCoord chunkCoords, glm::vec3 blockPos);

    /**
     * @brief Retrieves a block from the world using global world coordinates.
     *
     * @param worldPos The global world-space position
     * @return The corresponding Block, or an Air block if the position is outside loaded chunks or invalid.
     */
    Block getBlockAt(glm::vec3 worldPos);

private:
    std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>> chunkPositions;
    Shader &shader;
    TextureAtlas *textureAtlas;
};
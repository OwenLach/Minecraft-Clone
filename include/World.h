// #pragma once

// #include <unordered_map>
// #include <glm/glm.hpp>
// #include "Chunk.h"

// struct ChunkCoord
// {
//     int x;
//     int z;

//     bool operator==(const ChunkCoord &other) const;
// };

// namespace std
// {
//     template <>
//     struct hash<ChunkCoord>;
// }

// class World
// {
// public:
//     World();
//     ~World();

//     // Render all visible chunks.
//     // Push data to the GPU (VAO/VBO/etc.).
//     // Set shaders, textures, and draw the chunk meshes.
//     void render() const;

//     // Rebuild dirty chunk meshes (e.g. when a block has changed).
//     // Load/unload chunks as the player moves.
//     // Handle block updates, lighting updates, etc.
//     void update() const;

//     glm::vec3 chunkToWorldCoords(glm::vec3 chunkCoords) const;
//     glm::vec3 worldToChunkCoords(glm::vec3 worldCoords) const;
//     Chunk getChunk();

// private:
//     std::unordered_map<ChunkCoord, Chunk> chunkPositions;
// };
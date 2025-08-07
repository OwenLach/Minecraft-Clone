#pragma once

#include "Chunk/Vertex.h"
#include "Chunk/MeshData.h"
#include "OpenGL/VertexArray.h"
#include "OpenGL/VertexBuffer.h"
#include "OpenGL/ElementBuffer.h"
#include "OpenGL/VertexBufferLayout.h"

#include <vector>
#include <atomic>

class ChunkCoord;
class Shader;

class ChunkMesh
{
public:
    MeshData meshData_;
    size_t verticesCount_ = 0;
    size_t indicesCount_ = 0;
    std::atomic<bool> hasValidMesh_{false};

    ChunkMesh(Shader &chunkShader);
    void render(const ChunkCoord &coord);
    void uploadMesh();
    void setMeshValid();

private:
    VertexArray vao_;
    VertexBuffer vbo_;
    ElementBuffer ebo_;
    Shader &chunkShader_;
    void configureVertexAttributes();
};
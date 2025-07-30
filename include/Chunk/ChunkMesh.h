#pragma once

#include "Chunk/Vertex.h"
// #include "Chunk/ChunkMeshBuilder.h"
#include "OpenGL/VertexArray.h"
#include "OpenGL/VertexBuffer.h"
#include "OpenGL/ElementBuffer.h"
#include "OpenGL/VertexBufferLayout.h"

#include <vector>

class ChunkCoord;
class Shader;

class ChunkMesh
{
public:
    std::vector<Vertex> vertices_;
    std::vector<unsigned int> indices_;
    size_t verticesCount_ = 0;
    size_t indicesCount_ = 0;

    ChunkMesh(Shader &chunkShader);
    void render(const ChunkCoord &coord);
    void uploadMesh();

private:
    VertexArray vao_;
    VertexBuffer vbo_;
    ElementBuffer ebo_;
    Shader &chunkShader_;
    void configureVertexAttributes();
};
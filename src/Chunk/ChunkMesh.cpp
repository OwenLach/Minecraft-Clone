#include "Chunk/ChunkMesh.h"
#include "Chunk/ChunkCoord.h"
#include "Shader.h"
#include "Constants.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

ChunkMesh::ChunkMesh(Shader &chunkShader) : chunkShader_(chunkShader)
{
    const int chunkSize_X = Constants::CHUNK_SIZE_X;
    const int chunkSize_Y = Constants::CHUNK_SIZE_Y;
    const int chunkSize_Z = Constants::CHUNK_SIZE_Z;

    vertices_.reserve(chunkSize_X * chunkSize_Y * chunkSize_Z * 4); // Max 4 unique vertices per face
    indices_.reserve(chunkSize_X * chunkSize_Y * chunkSize_Z * 6);  // Max 6 indices per face

    configureVertexAttributes();
}

void ChunkMesh::render(const ChunkCoord &coord)
{
    if (verticesCount_ == 0 || indicesCount_ == 0)
    {
        return;
    }

    chunkShader_.use();
    vao_.bind();

    glm::mat4 model = glm::mat4(1.0f);
    auto modelMatrix_ = glm::translate(model, glm::vec3(coord.x * Constants::CHUNK_SIZE_X, 0, coord.z * Constants::CHUNK_SIZE_Z));
    chunkShader_.setMat4("model", modelMatrix_);

    glDrawElements(GL_TRIANGLES, indicesCount_, GL_UNSIGNED_INT, 0);
}

void ChunkMesh::uploadMesh()
{
    if (vertices_.empty() || indices_.empty())
    {
        std::cout << "TRIED TO UPLOAD EMPTY MESH" << std::endl;
        return;
    }

    vao_.bind();

    // Bind vertex buffer and set data
    vbo_.bind();
    verticesCount_ = vertices_.size();
    vbo_.setData(reinterpret_cast<const float *>(vertices_.data()), verticesCount_ * sizeof(Vertex));

    // Bind index buffer and set data
    ebo_.bind();
    indicesCount_ = indices_.size();
    ebo_.setData(indices_.data(), indicesCount_ * sizeof(unsigned int));

    vertices_.clear();
    indices_.clear();
}

void ChunkMesh::configureVertexAttributes()
{
    vao_.bind();
    vbo_.bind();
    ebo_.bind();

    VertexBufferLayout layout;
    layout.push<float>(3); // position
    layout.push<float>(2); // texture coords
    layout.push<float>(1); // AO
    vao_.addBuffer(vbo_, layout);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error in configureVertexAttributes: " << error << std::endl;
    }
}

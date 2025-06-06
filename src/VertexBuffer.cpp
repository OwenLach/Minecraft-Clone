#include "VertexBuffer.h"

#include <glad/glad.h>

#include <iostream>

VertexBuffer::VertexBuffer() : dataSet(false)
{
    glGenBuffers(1, &ID);
}

VertexBuffer::~VertexBuffer()
{
    if (ID != 0)
    {
        glDeleteBuffers(1, &ID);
    }
}

void VertexBuffer::setData(const float *data, int size)
{
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    dataSet = true;
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::bind() const
{
    if (!dataSet)
    {
        throw std::runtime_error("Attempting to bind VertexBuffer with no data");
    }
    glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VertexBuffer::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

#include "OpenGL/VertexBuffer.h"

#include <glad/glad.h>

#include <iostream>

VertexBuffer::VertexBuffer() : hasData_(false)
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
    hasData_ = true;
}

void VertexBuffer::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VertexBuffer::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

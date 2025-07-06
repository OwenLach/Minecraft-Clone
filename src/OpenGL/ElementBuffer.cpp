#include "OpenGL/ElementBuffer.h"

#include <glad/glad.h>

#include <iostream>

ElementBuffer::ElementBuffer() : hasData_(false)
{
    glGenBuffers(1, &ID);
}

ElementBuffer::~ElementBuffer()
{
    if (ID != 0)
    {
        glDeleteBuffers(1, &ID);
    }
}

void ElementBuffer::setData(const unsigned int *data, int size)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    hasData_ = true;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ElementBuffer::bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}

void ElementBuffer::unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

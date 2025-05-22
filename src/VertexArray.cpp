#include "VertexArray.h"
#include <glad/glad.h>

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &ID);
    glBindVertexArray(ID);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &ID);
}

void VertexArray::addBuffer(VertexBuffer &vb, VertexBufferLayout &layout)
{
    bind();
    vb.bind();

    unsigned int i = 0;
    unsigned int offset = 0;
    for (const auto &attribute : layout.getAttributes())
    {
        glVertexAttribPointer(i, attribute.count, attribute.type, attribute.normalized, layout.getStride(), reinterpret_cast<const void *>(static_cast<uintptr_t>(offset)));

        glEnableVertexAttribArray(i);

        offset += attribute.count * VertexBufferAttribute::getSizeOfType(attribute.type);
        i++;
    }
}

void VertexArray::bind() const
{
    glBindVertexArray(ID);
}

void VertexArray::unbind() const
{
    glBindVertexArray(0);
}
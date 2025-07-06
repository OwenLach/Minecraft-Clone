#pragma once

#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

class VertexArray
{
public:
    VertexArray();
    ~VertexArray();
    void addBuffer(VertexBuffer &vb, VertexBufferLayout &layout);
    void bind() const;
    void unbind() const;

private:
    unsigned int ID;
};
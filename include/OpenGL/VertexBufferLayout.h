#pragma once

#include <glad/glad.h>
#include <vector>

/**
 * @brief Represents a single element (attribute) in a vertex buffer layout.
 *
 * Each element describes the type, count, and normalization of a vertex attribute.
 */
struct VertexBufferAttribute
{
    unsigned int count;
    unsigned int type;
    unsigned char normalized;

    static unsigned int getSizeOfType(unsigned int type)
    {
        switch (type)
        {
        case GL_FLOAT:
            return 4;
        case GL_UNSIGNED_INT:
            return 4;
        case GL_UNSIGNED_BYTE:
            return 1;
        default:
            return 0;
        }
    }
};

/**
 * @brief Describes the layout of vertex data within a Vertex Buffer Object (VBO).
 *
 * This class defines how the GPU should interpret the vertex data,
 * such as the type, count, and normalization of each attribute.
 */
class VertexBufferLayout
{
private:
    std::vector<VertexBufferAttribute> attributes;
    unsigned int stride = 0;

public:
    template <typename T>
    void push(unsigned int count)
    {
        static_assert(sizeof(T) == 0, "type not supported");
    }

    const std::vector<VertexBufferAttribute> &getAttributes() const { return attributes; }
    unsigned int getStride() const { return stride; }
};

template <>
inline void VertexBufferLayout::push<float>(unsigned int count)
{
    attributes.push_back({count, GL_FLOAT, GL_FALSE});
    stride += count * VertexBufferAttribute::getSizeOfType(GL_FLOAT);
}

template <>
inline void VertexBufferLayout::push<unsigned int>(unsigned int count)
{
    attributes.push_back({count, GL_UNSIGNED_INT, GL_FALSE});
    stride += count * VertexBufferAttribute::getSizeOfType(GL_UNSIGNED_INT);
}

template <>
inline void VertexBufferLayout::push<unsigned char>(unsigned int count)
{
    attributes.push_back({count, GL_UNSIGNED_BYTE, GL_TRUE});
    stride += count * VertexBufferAttribute::getSizeOfType(GL_UNSIGNED_BYTE);
}
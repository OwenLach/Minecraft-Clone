#pragma once

/**
 * @brief A buffer that holds vertex data and manages its lifecycle on the GPU.
 *
 * This class encapsulates an OpenGL Vertex Buffer Object (VBO) and
 * provides methods to bind and unbind the buffer.
 *
 * @note This class does not interpret the contents or structure of the vertex data.
 *       It simply stores raw data and uploads it to the GPU. It is the user's responsibility
 *       to ensure the data format matches the shader inputs and vertex attribute setup.
 */
class VertexBuffer
{
public:
    /**
     * @brief Constructs a VertexBuffer and uploads data to the GPU.
     */
    VertexBuffer();

    /**
     * @brief Destructor that deletes the OpenGL buffer.
     */
    ~VertexBuffer();

    void setData(const float *data, int size);
    /**
     * @brief Binds the VertexBuffer (makes it the active GL_ARRAY_BUFFER).
     */
    void bind() const;

    /**
     * @brief Unbinds the VertexBuffer (binds zero to GL_ARRAY_BUFFER).
     */
    void unbind() const;

private:
    bool dataSet;
    unsigned int ID;
};
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
     * @param data Pointer to an array of floats representing vertex data.
     * @param size Size of the data in bytes.
     */
    VertexBuffer(const float *data, int size);

    /**
     * @brief Destructor that deletes the OpenGL buffer.
     */
    ~VertexBuffer();

    /**
     * @brief Binds the VertexBuffer (makes it the active GL_ARRAY_BUFFER).
     */
    void bind() const;

    /**
     * @brief Unbinds the VertexBuffer (binds zero to GL_ARRAY_BUFFER).
     */
    void unbind() const;

private:
    unsigned int ID;
};
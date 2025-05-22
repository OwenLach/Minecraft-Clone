#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader
{
public:
    unsigned int ID;

    /**
     * Constructs a Shader object by compiling and linking a vertex and fragment shader.
     *
     * @param vertexPath   File path to the vertex shader source code.
     * @param fragmentPath File path to the fragment shader source code.
     */
    Shader(const char *vertexPath, const char *fragmentPath);

    /**
     * Activates the shader program for use in the current OpenGL rendering state.
     * This must be called before setting uniforms or rendering with this shader.
     */
    void use() const;

    /**
     * Sets a boolean uniform in the shader.
     *
     * @param name  The name of the uniform variable.
     * @param value The boolean value to set.
     */
    void setBool(const std::string &name, bool value) const;

    /**
     * Sets an integer uniform in the shader.
     *
     * @param name  The name of the uniform variable.
     * @param value The integer value to set.
     */
    void setInt(const std::string &name, int value) const;

    /**
     * Sets a float uniform in the shader.
     *
     * @param name  The name of the uniform variable.
     * @param value The float value to set.
     */
    void setFloat(const std::string &name, float value) const;

    /**
     * Sets a vec2 uniform in the shader using a glm::vec2.
     *
     * @param name  The name of the uniform variable.
     * @param value The 2D vector to set.
     */
    void setVec2(const std::string &name, const glm::vec2 &value) const;

    /**
     * Sets a vec2 uniform in the shader using individual float components.
     *
     * @param name The name of the uniform variable.
     * @param x    The x component of the vector.
     * @param y    The y component of the vector.
     */
    void setVec2(const std::string &name, float x, float y) const;

    /**
     * Sets a vec3 uniform in the shader using a glm::vec3.
     *
     * @param name  The name of the uniform variable.
     * @param value The 3D vector to set.
     */
    void setVec3(const std::string &name, const glm::vec3 &value) const;

    /**
     * Sets a vec3 uniform in the shader using individual float components.
     *
     * @param name The name of the uniform variable.
     * @param x    The x component of the vector.
     * @param y    The y component of the vector.
     * @param z    The z component of the vector.
     */
    void setVec3(const std::string &name, float x, float y, float z) const;

    /**
     * Sets a vec4 uniform in the shader using a glm::vec4.
     *
     * @param name  The name of the uniform variable.
     * @param value The 4D vector to set.
     */
    void setVec4(const std::string &name, const glm::vec4 &value) const;

    /**
     * Sets a vec4 uniform in the shader using individual float components.
     *
     * @param name The name of the uniform variable.
     * @param x    The x component of the vector.
     * @param y    The y component of the vector.
     * @param z    The z component of the vector.
     * @param w    The w component of the vector.
     */
    void setVec4(const std::string &name, float x, float y, float z, float w) const;

    /**
     * Sets a mat2 uniform in the shader.
     *
     * @param name The name of the uniform variable.
     * @param mat  The 2x2 matrix to set.
     */
    void setMat2(const std::string &name, const glm::mat2 &mat) const;

    /**
     * Sets a mat3 uniform in the shader.
     *
     * @param name The name of the uniform variable.
     * @param mat  The 3x3 matrix to set.
     */
    void setMat3(const std::string &name, const glm::mat3 &mat) const;

    /**
     * Sets a mat4 uniform in the shader.
     *
     * @param name The name of the uniform variable.
     * @param mat  The 4x4 matrix to set.
     */
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    /**
     * Checks for shader compilation or program linking errors and prints them to the console.
     *
     * @param shader The OpenGL shader or program ID to check.
     * @param type   A string indicating the type of object ("VERTEX", "FRAGMENT", or "PROGRAM").
     *               Determines whether to check compile or link status.
     */
    void checkCompileErrors(GLuint shader, std::string type);
};

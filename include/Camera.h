#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <memory>

#include "Constants.h"
#include "Chunk.h"
#include "Shader.h"

enum CameraMovement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

struct Frustum
{
    std::array<glm::vec4, 6> planes;
};

class Camera
{
public:
    // camera attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    float ZoomSensitivity;

    Frustum frustum;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = Constants::YAW,
           float pitch = Constants::PITCH);

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix();

    void ProcessKeyboard(CameraMovement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

    bool isAABBInFrustum(BoundingBox &boundingBox) const;

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors();
    void updateFrustum();
};
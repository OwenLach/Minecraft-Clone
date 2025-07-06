#include "Camera.h"
#include "Constants.h"
#include "Shader.h"
#include "OpenGL/VertexArray.h"
#include "OpenGL/VertexBuffer.h"
#include "OpenGL/VertexBufferLayout.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

Camera::Camera(const glm::vec3 position, const glm::vec3 up, float yaw, float pitch)
    : Position(position),
      Front(0.0f, 0.0f, -1.0f),
      Up(0.0f),
      Right(0.0f),
      WorldUp(up),
      Yaw(yaw),
      Pitch(pitch),
      MovementSpeed(Constants::PLAYER_SPEED),
      MouseSensitivity(Constants::MOUSE_SENSITIVITY),
      Zoom(Constants::ZOOM),
      ZoomSensitivity(Constants::ZOOM_SENSITIVITY)
{
    updateCameraVectors();
    updateFrustum();
}

glm::mat4 Camera::getViewMatrix()
{
    return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::getProjectionMatrix()
{
    return glm::perspective(glm::radians(Zoom), (float)Constants::SCREEN_W / (float)Constants::SCREEN_H, 0.1f, 1000.0f);
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    switch (direction)
    {
    case CameraMovement::FORWARD:
        Position += Front * velocity;
        break;
    case CameraMovement::BACKWARD:
        Position -= Front * velocity;
        break;
    case CameraMovement::LEFT:
        Position -= Right * velocity;
        break;
    case CameraMovement::RIGHT:
        Position += Right * velocity;
        break;
    case CameraMovement::UP:
        Position += Up * velocity;
        break;
    case CameraMovement::DOWN:
        Position -= Up * velocity;
        break;
    }

    updateFrustum();
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
    updateFrustum();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    Zoom -= (float)yoffset * ZoomSensitivity;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 90.0f)
        Zoom = 90.0f;

    updateFrustum();
}

bool Camera::isAABBInFrustum(const BoundingBox &boundingBox) const
{
    glm::vec3 min = boundingBox.min;
    glm::vec3 max = boundingBox.max;

    const float epsilon = 0.1f;

    for (int i = 0; i < 5; i++)
    {
        auto &plane = frustum.planes[i];
        float x = (plane.x >= 0) ? max.x : min.x;
        float y = (plane.y >= 0) ? max.y : min.y;
        float z = (plane.z >= 0) ? max.z : min.z;
        glm::vec3 normal = glm::vec3(plane);
        float distance = glm::dot(normal, glm::vec3(x, y, z)) + plane.w;

        if (distance < -epsilon)
            return false;
    }

    return true;
}

void Camera::updateCameraVectors()
{
    // calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, WorldUp)); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up = glm::normalize(glm::cross(Right, Front));
}

void Camera::updateFrustum()
{
    glm::mat4 vp = getProjectionMatrix() * getViewMatrix();

    glm::vec4 row0 = glm::vec4(vp[0][0], vp[1][0], vp[2][0], vp[3][0]);
    glm::vec4 row1 = glm::vec4(vp[0][1], vp[1][1], vp[2][1], vp[3][1]);
    glm::vec4 row2 = glm::vec4(vp[0][2], vp[1][2], vp[2][2], vp[3][2]);
    glm::vec4 row3 = glm::vec4(vp[0][3], vp[1][3], vp[2][3], vp[3][3]);

    frustum.planes[0] = row3 + row0; // Left
    frustum.planes[1] = row3 - row0; // Right
    frustum.planes[2] = row3 + row1; // Bottom
    frustum.planes[3] = row3 - row1; // Top
    frustum.planes[4] = row3 + row2; // Near
    frustum.planes[5] = row3 - row2; // Far

    for (auto &plane : frustum.planes)
    {
        float length = glm::length(glm::vec3(plane));
        if (length > 0.0f)
            plane /= length;
    }
}

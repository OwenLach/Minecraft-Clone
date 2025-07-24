#include "InputManager.h"
#include "Constants.h"
#include "World.h"

#include <GLFW/glfw3.h>
#include <iostream>

InputManager::InputManager(Camera &camera, GLFWwindow *window, World &world)
    : camera_(camera), window_(window), world_(world), firstMouse_(true), lastX_(Constants::SCREEN_W / 2), lastY_(Constants::SCREEN_H / 2)
{
    setInputCallbacks();
}

void InputManager::setInputCallbacks()
{
    // set camera to the window
    glfwSetWindowUserPointer(window_, this);
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetFramebufferSizeCallback(window_, InputManager::framebuffer_size_callback);
    glfwSetCursorPosCallback(window_, InputManager::mouse_callback);
    glfwSetMouseButtonCallback(window_, InputManager::mouse_button_callback);
    glfwSetScrollCallback(window_, InputManager::scroll_callback);
}

void InputManager::processKeyboard(GLFWwindow *window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera_.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera_.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera_.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera_.ProcessKeyboard(DOWN, deltaTime);
}

void InputManager::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void InputManager::mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    // get input manager pointer from the glfw window
    InputManager *inputManager = static_cast<InputManager *>(glfwGetWindowUserPointer(window));
    if (!inputManager)
        return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (inputManager->firstMouse_)
    {
        inputManager->lastX_ = xpos;
        inputManager->lastY_ = ypos;
        inputManager->firstMouse_ = false;
    }

    float xoffset = xpos - inputManager->lastX_;
    float yoffset = inputManager->lastY_ - ypos;
    inputManager->lastX_ = xpos;
    inputManager->lastY_ = ypos;

    inputManager->camera_.ProcessMouseMovement(xoffset, yoffset);
}

void InputManager::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    InputManager *inputManager = static_cast<InputManager *>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        inputManager->world_.breakBlock();
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        inputManager->world_.placeBlock();
    }
}

void InputManager::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    InputManager *inputManager = static_cast<InputManager *>(glfwGetWindowUserPointer(window));
    if (!inputManager)
        return;

    inputManager->camera_.ProcessMouseScroll(static_cast<float>(yoffset));
}

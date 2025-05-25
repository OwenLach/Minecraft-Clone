#include "InputManager.h"
#include "Constants.h"

InputManager::InputManager(Camera *camera)
    : camera(camera), firstMouse(true), lastX(Constants::SCREEN_W / 2), lastY(Constants::SCREEN_H / 2)
{
}

void InputManager::processKeyboard(GLFWwindow *window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera->ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera->ProcessKeyboard(DOWN, deltaTime);
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

    if (inputManager->firstMouse)
    {
        inputManager->lastX = xpos;
        inputManager->lastY = ypos;
        inputManager->firstMouse = false;
    }

    float xoffset = xpos - inputManager->lastX;
    float yoffset = inputManager->lastY - ypos;
    inputManager->lastX = xpos;
    inputManager->lastY = ypos;

    if (inputManager->camera)
        inputManager->camera->ProcessMouseMovement(xoffset, yoffset);
}

void InputManager::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    InputManager *inputManager = static_cast<InputManager *>(glfwGetWindowUserPointer(window));
    if (!inputManager)
        return;

    if (inputManager->camera)
        inputManager->camera->ProcessMouseScroll(static_cast<float>(yoffset));
}

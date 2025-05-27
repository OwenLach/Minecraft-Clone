#include "Application.h"
#include "Constants.h"
#include "Shader.h"
#include "VertexArray.h"
#include "TextureAtlas.h"
#include "BlockTypes.h"
#include "Block.h"
#include "Chunk.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

Application::Application()
    : camera(glm::vec3(0.0f, 0.0f, 3.0f)), inputManager(&camera)
{
}

void Application::run()
{
    if (!initWindow())
    {
        std::cerr << "Failed to initialize GLFW window" << std::endl;
        return;
    }

    if (!initOpenGL())
    {
        std::cerr << "Failed to initialize OpenGL" << std::endl;
        return;
    }

    render();
}

bool Application::initWindow()
{
    // initialize and configure glfw window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // window creation
    window = glfwCreateWindow(Constants::SCREEN_W, Constants::SCREEN_H, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    // set camera to the window
    glfwSetWindowUserPointer(window, &inputManager);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetFramebufferSizeCallback(window, InputManager::framebuffer_size_callback);
    glfwSetCursorPosCallback(window, InputManager::mouse_callback);
    glfwSetScrollCallback(window, InputManager::scroll_callback);

    return true;
}

bool Application::initOpenGL()
{
    // load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        return false;
    }

    return true;
}

void Application::render()
{
    Shader shader("../shaders/vShader.glsl", "../shaders/fShader.glsl");
    TextureAtlas textureAtlas;
    Chunk chunk(shader, &textureAtlas);

    float lastFrame = 0;
    // render loop

    while (!glfwWindowShouldClose(window))
    {
        // calculate delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        inputManager.processKeyboard(window, deltaTime);

        // render
        glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // enable depth testing and cull facing
        glEnable(GL_DEPTH_TEST);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK); // Cull back-facing triangles
        glFrontFace(GL_CCW); // Counter-clockwise is front

        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureAtlas.ID);

        // activate shader
        shader.use();

        // pass projection matrix to shader (note as projection matricies rarely change, there's no need to do this per frame)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)Constants::SCREEN_W / (float)Constants::SCREEN_H, 0.1f, 100.0f);
        shader.setMat4("projection", projection);

        // set the view matrix
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", view);

        chunk.renderChunk();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
}
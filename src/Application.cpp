#include "Application.h"
#include "Constants.h"
#include "VertexArray.h"
#include "TextureAtlas.h"
#include "BlockTypes.h"
#include "Block.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

Application::Application()
    : lastFrame(0.0f),
      fpsTimer(0.0f),
      frameCount(0),
      fpsToDisplay(0.0f)
{
}

void Application::run()
{

    if (!init())
    {
        std::cerr << "Failed to initialize application!" << std::endl;
        return;
    }
    mainLoop();
    shutdown();
}

bool Application::init()
{
    if (!initWindow() || !initOpenGL())
    {
        std::cerr << "Error initializing GLFW or OpenGl" << std::endl;
        return false;
    }

    camera = std::make_unique<Camera>(glm::vec3(0.0f));
    inputManager = std::make_unique<InputManager>(camera.get());

    setupInputCallbacks();

    textureAtlas = std::make_unique<TextureAtlas>();
    shader = std::make_unique<Shader>("../shaders/vShader.glsl", "../shaders/fShader.glsl");
    imguiManager = std::make_unique<ImGuiManager>(window);
    world = std::make_unique<World>(*shader, textureAtlas.get());
    crosshair = std::make_unique<Crosshair>();
    return true;
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

void Application::shutdown()
{
    glfwTerminate();
}

void Application::mainLoop()
{

    while (!glfwWindowShouldClose(window))
    {

        float dt = getDeltaTime();

        processInput(dt);
        update(dt);
        render();
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Application::render()
{
    setGLRenderState();
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureAtlas->ID);

    // activate shader
    shader->use();

    // pass projection matrix to shader (note as projection matricies rarely change, there's no need to do this per frame)
    glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)Constants::SCREEN_W / (float)Constants::SCREEN_H, 0.1f, 1000.0f);
    shader->setMat4("projection", projection);

    // set the view matrix
    glm::mat4 view = camera->GetViewMatrix();
    shader->setMat4("view", view);

    world->render();
    crosshair->render();
    imguiManager->render();
}

void Application::update(const float dt)
{
    updateFPS(dt);
    imguiManager->update();
    setupImGuiUI();
}

void Application::setGLRenderState()
{
    // enable depth testing and cull facing
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    // Cull back-facing triangles
    glCullFace(GL_BACK);
    // Counter-clockwise is front
    glFrontFace(GL_CCW);
}

void Application::setupInputCallbacks()
{
    // set camera to the window
    glfwSetWindowUserPointer(window, inputManager.get());
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetFramebufferSizeCallback(window, InputManager::framebuffer_size_callback);
    glfwSetCursorPosCallback(window, InputManager::mouse_callback);
    glfwSetScrollCallback(window, InputManager::scroll_callback);
}

float Application::getDeltaTime()
{
    float currentFrame = static_cast<float>(glfwGetTime());
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    return deltaTime;
}

void Application::processInput(float dt)
{
    inputManager->processKeyboard(window, dt);
}

void Application::updateFPS(const float dt)
{
    frameCount++;
    fpsTimer += dt;
    if (fpsTimer >= 0.5f) // update every 0.5 seconds
    {
        fpsToDisplay = frameCount / fpsTimer;
        fpsTimer = 0.0f;
        frameCount = 0;
    }
}

void Application::setupImGuiUI()
{
    ImGui::Begin("Stats");
    ImGui::SetWindowSize(ImVec2(300, 100));
    ImGui::Text("FPS: %.1f", fpsToDisplay);
    glm::vec3 camPos = camera->Position;
    ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
    float zoom = camera->Zoom;
    ImGui::Text("FOV: (%.2f)", zoom);
    ImGui::Text("Chunks Rendered: %zu", world->ChunksRendered);
    ImGui::End();
}

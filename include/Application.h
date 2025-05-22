#include "Camera.h"
#include "InputManager.h"

struct GLFWwindow;

class Application
{
public:
    Application();
    void run();

private:
    GLFWwindow *window;
    Camera camera;
    InputManager inputManager;

    bool initWindow();
    bool initOpenGL();
    void render();
};
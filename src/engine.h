#ifndef FACTORY_ENGINE_H
#define FACTORY_ENGINE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "util.h"

using namespace util;

class engine {
public:
    engine() {
        int res = init();

        if (res != 0) {
            glfwTerminate();
            log(ERRO, "INIT FAILED");
        }
    };
private:
    int init();

    GLFWwindow* window;
    glm::ivec2 winSize;

    double startup = 0;
    double lastframe = 0;


    unsigned int shader_program;

    //uniforms
    unsigned int viewLoc;
    unsigned int projLoc;
    unsigned int camPosLoc;
};

#endif
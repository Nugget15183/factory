#include "engine.h"
#include "util.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "stb_image.h"

using namespace util;

void setWindowCallbacks(GLFWwindow* window) {
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        engine* ptr = (engine*)glfwGetWindowUserPointer(window);

        ptr->winSize = { width, height };

        glViewport(0, 0, width, height);
    });
}

int engine::init() {
    log(INFO, "Initalizing...");

    if (!glfwInit()) {log(ERRO, "GLFW Failed to initalize"); return -1;}

    //window hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //window creation
    window = glfwCreateWindow(winSize[0],  winSize[1], "Factory Game", NULL, NULL);
    if (window == nullptr) {log(ERRO, "GLFW Failed to create a window"); glfwTerminate(); return -1;}
    glfwMakeContextCurrent(window);

    //load glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {log(ERRO, "GLAD Failed to load"); return -1;}

    glViewport(0, 0, winSize[0], winSize[1]);//size of the window
    setWindowCallbacks(window);

    //Shader initalization
    string rv = readFile("resources/main.vert");
    string rf =  readFile("resources/main.frag");
    const GLchar *vert_src = rv.c_str();
    const GLchar *frag_src =rf.c_str();

    unsigned int vert_shader=glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vert_src, NULL);
    glCompileShader(vert_shader);

    unsigned int frag_shader=glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &frag_src, NULL);
    glCompileShader(frag_shader);

    //confirm vertex shader compiled successfully
    int vsucc;
    char vinfo_log[512];
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &vsucc);
    if (!vsucc) {
        glGetShaderInfoLog(vert_shader, sizeof(vinfo_log), NULL, vinfo_log);
        log(ERRO, ("Failed to compile vertex shader: " + string(vinfo_log)).c_str());
        return -1;
    }

    //confirm frag shader compiled successfully
    int fsucc;
    char finfo_log[512];
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &fsucc);
    if (!fsucc) {
        glGetShaderInfoLog(frag_shader, sizeof(finfo_log), NULL, finfo_log);
        log(ERRO, ("Failed to compile fragment shader: " + string(finfo_log)).c_str());
        return -1;
    }

    //create shader program and link
    shader_program=glCreateProgram();
    glAttachShader(shader_program, vert_shader);
    glAttachShader(shader_program, frag_shader);
    glLinkProgram(shader_program);

    //link shaders and confirm success
    int lsucc;
    char linfo_log[512];
    glGetProgramiv(shader_program, GL_LINK_STATUS, &lsucc);
    if (!lsucc) {
        glGetProgramInfoLog(shader_program, sizeof(linfo_log), NULL, linfo_log);
        log(ERRO, ("Failed to link shaders: " + string(linfo_log)).c_str());
        return -1;
    }

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    //get uniform locs
    viewLoc = glGetUniformLocation(shader_program, "view");
    projLoc = glGetUniformLocation(shader_program, "proj");
    modelLoc = glGetUniformLocation(shader_program, "model");

    camPosLoc = glGetUniformLocation(shader_program, "camPosition");

    //opengl settings and misc stuff
    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, this);

    log(INFO, "Initalized successfully");

    double time = glfwGetTime();

    startup=time;
    lastframe=time;

    cube = mesh::cube();
    cube.create();

    return 0;
}

void engine::close() {
    log(INFO, "Closing...");

    glfwDestroyWindow(window);
    glfwTerminate();

    log(INFO, ("Session duration: " + to_string(glfwGetTime() - startup)).c_str());
}

void engine::update() {
    glfwPollEvents();
}

void engine::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_program);

    float aspect = winSize.y > 0 ? (float)winSize[0] / (float)winSize[1] : 1.0f;
    glm::mat4 proj = glm::perspective(glm::radians(FOV), aspect, 0.01f, 1000.0f);
    glm::mat4 view = glm::lookAt(camPos, camTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0, 1, 0));

    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(camPosLoc, 1, glm::value_ptr(camPos));

    cube.draw();

    glfwSwapBuffers(window);
}
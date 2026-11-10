#include "engine.h"
#include "util.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "stb_image.h"

using namespace util;

void setWindowCallbacks(GLFWwindow* window) {

}

int engine::init() {
    log(INFO, "Initalizing...");

    if (!glfwInit()) {log(ERRO, "GLFW Failed to initalize"); return -1;}

    //window hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //window creation
    window = glfwCreateWindow(800, 600, "Factory Game", NULL, NULL);
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

    //get uniform locs
    viewLoc = glGetUniformLocation(shader_program, "view");
    projLoc = glGetUniformLocation(shader_program, "proj");
    camPosLoc = glGetUniformLocation(shader_program, "camPosition");

    //opengl settings and misc stuff
    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, this);

    log(INFO, "Initalized successfully");

    double time = glfwGetTime();

    startup=time;
    lastframe=time;

    return 0;
}

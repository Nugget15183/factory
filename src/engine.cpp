#include "engine.h"
#include "util.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "stb_image.h"

using namespace util;

static const glm::vec4 input_col(0.2f, 0.5f, 1.0f, 1.0f);
static const glm::vec4 output_col(1.0f, 0.6f, 0.1f, 1.0f);

void setWindowCallbacks(GLFWwindow* window) {
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        engine* ptr = (engine*)glfwGetWindowUserPointer(window);

        ptr->winSize = { width, height };

        glViewport(0, 0, width, height);
    });
}

void loadWorld(engine* e) {
    double world_start = glfwGetTime();

    std::vector<tileInst> inst;
    inst.reserve(e->worldSize.x * e->worldSize.y);

    e->w = new world(e->worldSize.x, e->worldSize.y);
    log(INFO, ("World loaded successfully in " + to_string(glfwGetTime() - world_start) + "s").c_str());

    double obj_start = glfwGetTime();
    log(INFO, "Loading objects...");

    object grid = object("resources/meshes/grid.obj");
    grid.create(e->genHandle());

    grid.props.unlit=true;
    grid.props.col=glm::vec4(0.8f, 0.8f, 0.8f, 0.5f);
    grid.props.visible=false;

    e->renderables.insert({grid.id, grid});

    for (int x = -e->worldSize.x / 2; x < -e->worldSize.x / 2 + e->worldSize.x; x++) {
        for (int y = -e->worldSize.y / 2; y < -e->worldSize.y / 2 + e->worldSize.y; y++) {
            glm::vec4 col = {0,0,0,1};
            cellType c = e->w->getCell({x,y});
            switch (c) {
                case GROUND: {
                    col = {0.23f, 0.16f, 0.13f, 1.0f};
                    break;
                } case IRON: {
                    col = {0.3f, 0.31f, 0.33f, 1.0f};
                    break;
                } case COAL: {
                    col = {0.2f, 0.2f, 0.2f, 1.0f};
                    break;
                } case COPPER: {
                    col = {0.72f, 0.45f, 0.2f, 1.0f};
                    break;
                } case OBSTRUCTED: {
                    col = {0,0,0,1};
                }
            }

            inst.push_back({glm::vec3(x, 0, y), col});
        }
    }

    e->tileMesh = mesh::cube();
    e->tileMesh.create();
    e->tilecount=(int)inst.size();

    glGenBuffers(1, &e->tileInstVBO);
    glBindBuffer(GL_ARRAY_BUFFER, e->tileInstVBO);
    glBufferData(GL_ARRAY_BUFFER, inst.size() * sizeof(tileInst), inst.data(), GL_STATIC_DRAW);

    glBindVertexArray(e->tileMesh.VAO);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(tileInst), (void*)offsetof(tileInst, pos));
    glVertexAttribDivisor(3, 1);

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(tileInst), (void*)offsetof(tileInst, col));
    glVertexAttribDivisor(4, 1);

    glBindVertexArray(0);

    log(INFO, ("Loaded objects in " + to_string(glfwGetTime() - obj_start) + "s").c_str());
}

void checkKeyInputs(engine* e) {
    GLFWwindow &w = e->getWindow();

    if (glfwGetKey(&w, GLFW_KEY_ESCAPE) == GLFW_PRESS) {glfwSetWindowShouldClose(&w, true);}

    glm::vec2 input = {};

    if (glfwGetKey(&w, GLFW_KEY_W) == GLFW_PRESS) {input.y=e->cameraSpeed;}
    if (glfwGetKey(&w, GLFW_KEY_A) == GLFW_PRESS) {input.x=-e->cameraSpeed;}
    if (glfwGetKey(&w, GLFW_KEY_S) == GLFW_PRESS) {input.y=-e->cameraSpeed;}
    if (glfwGetKey(&w, GLFW_KEY_D) == GLFW_PRESS) {input.x=e->cameraSpeed;}

    if (glm::length(input) > 0) {
       input = glm::normalize(input);

        glm::vec3 fw = e->cameraTargetOffset;
        fw.y=0;
        fw=glm::normalize(fw);

        glm::vec3 rt = glm::normalize(glm::cross(fw, glm::vec3(0, 1, 0)));

        glm::vec3 move = (fw * input.y + rt * input.x)  * e->cameraSpeed * (float)e->dt;
        e->camPos+=move;
    }

    e->camTarget = e->camPos + e->cameraTargetOffset;
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

    //ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    //Shader initalization
    string rv = readFile("resources/main.vert");
    string rf =  readFile("resources/main.frag");
    const GLchar *vert_src = rv.c_str();
    const GLchar *frag_src = rf.c_str();

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

    colLoc = glGetUniformLocation(shader_program, "baseCol");
    litLoc = glGetUniformLocation(shader_program, "unlit");
    camPosLoc = glGetUniformLocation(shader_program, "camPosition");

    instLoc = glGetUniformLocation(shader_program, "instanced");

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
    lastframeT=time;

    log(INFO, "Loading world...");
    loadWorld(this);

    return 0;
}

void engine::close() {
    log(INFO, "Closing...");

    double duration = glfwGetTime() - startup;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    log(INFO, "--- Session Info ---");
    log(INFO, ("Session duration: " + to_string(duration)).c_str());
    log(INFO, ("Average FPS: " + to_string(floor(frames / duration))).c_str());
}

void engine::drawPorts(objectType t, glm::ivec2 pos, direction dir, float alpha) {
    const buildingDef &d = defOf(t);
    glm::ivec2 f = dirVec(dir);

    float y = 0.5f + d.height * 0.5f;

    glm::vec3 base(pos.x, y, pos.y);
    glm::vec3 size(0.3f);

    if (d.hasOutput) {
        glm::vec4 col = output_col;
        col.a = alpha;

        drawCube(base + glm::vec3(f.x, 0, f.y) * 0.5f, size, 0.0f, col, true);
    }

    if (d.hasInput) {
        glm::vec4 col = input_col;
        col.a = alpha;

        drawCube(base - glm::vec3(f.x, 0, f.y) * 0.5f, size, 0.0f, col, true);
    }
}

void engine::drawUI() {
    ImGui::SetNextWindowPos(ImVec2((float)winSize.x * 0.5f, (float)winSize.y - 10), ImGuiCond_Always, ImVec2(0.5f, 1.0f));

    ImGui::Begin("Hotbar", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoScrollbar
    );

    for (int i = 0; i < OBJECT_COUNT; i++) {
        const buildingDef &d = defOf((objectType)i);

        if (i) ImGui::SameLine();

        ImGui::PushID(i);

        ImVec4 col(d.col.r, d.col.g, d.col.b, 1.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, selected == i ? col : ImVec4(col.x * 0.5f, col.y * 0.5f, col.z * 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, col);

        if (ImGui::Button(d.name, ImVec2(80, 44))) {
            selected = selected == i ? -1 : i;
        }

        ImGui::PopStyleColor(2);
        ImGui::PopID();
    }

    ImGui::Text("LMB place | RMB remove | R rotate | 1-%d select | Q deselect", (int)OBJECT_COUNT);
    ImGui::End();
}

void engine::handlePlacement() {
    hoverValid = false;
    if (ImGui::GetIO().WantCaptureMouse) return;

    //normalize cursor pos
    double mx, my;
    int ww, wh;
    glfwGetWindowSize(window, &ww, &wh);
    glfwGetCursorPos(window, &mx, &my);

    if (ww <= 0 || wh <= 0) return;

    float nx = 2.0f * (float)mx / ww - 1.0f;
    float ny = 1.0f - 2.0f * (float)my / wh;

    glm::mat4 inv = glm::inverse(projM * viewM);

    glm::vec4 a = inv * glm::vec4(nx, ny, -1, 1);
    glm::vec4 b = inv * glm::vec4(nx, ny, 1, 1);

    glm::vec3 p0 = glm::vec3(a)/a.w;
    glm::vec3 p1 = glm::vec3(b)/b.w;

    glm::vec3 dir = glm::normalize(p1 - p0);

    if (dir.y >= -1e-5f) return; //check if ray points away from ground

    float t = (0.5f - p0.y) / dir.y;
    glm::vec3 hit = p0 + dir * t;

    hoverCell = {
        (int)floor(hit.x + 0.5f),
        (int)floor(hit.z + 0.5f),
    };

    hoverValid = w->inBounds(hoverCell);
    if (!hoverValid) return;

    if (selected >= 0 && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        w->place((objectType)selected, hoverCell, placeDir);
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        w->remove(hoverCell);
    }

    bool r = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
    if (r && !prevR) placeDir = (direction)(((int)placeDir + 1) % 4);
    prevR = r;

    for (int i = 0; i < OBJECT_COUNT; i++) {
        if (glfwGetKey(window, GLFW_KEY_1 + i) == GLFW_PRESS) {
            selected = i;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        selected = -1;
    }
}

void engine::drawCube(glm::vec3 pos, glm::vec3 scale, float yawDeg, glm::vec4 col, bool unlit) {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos);
    m = glm::rotate(m, glm::radians(yawDeg), glm::vec3(0, 1, 0));
    m = glm::scale(m, scale);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
    glUniform4fv(colLoc, 1, glm::value_ptr(col));
    glUniform1i(litLoc, unlit);
    tileMesh.draw();
}

void engine::update() {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    handlePlacement();
    drawUI();

    w->update(dt);

    frames++;

    if (glfwGetTime() - lastframe >= 1.0f) {
        lastframe = glfwGetTime();

        int fps = frames - lastframec;
        lastframec = frames;

        glfwSetWindowTitle(window, string("Factory -:- FPS " + to_string(fps)).c_str());
    }

    checkKeyInputs(this);

    dt = glfwGetTime() - lastframeT;
    lastframeT = glfwGetTime();
}

void engine::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_program);

    float aspect = winSize.y > 0 ? (float)winSize[0] / (float)winSize[1] : 1.0f;
    glm::mat4 proj = glm::perspective(glm::radians(FOV), aspect, 0.01f, 1000.0f);
    glm::mat4 view = glm::lookAt(camPos, camTarget, glm::vec3(0.0f, 1.0f, 0.0f));

    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniform3fv(camPosLoc, 1, glm::value_ptr(camPos));

    projM = proj;
    viewM = view;

    //draw instanced ground
    glUniform1i(instLoc, 1);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
    glUniform1i(litLoc, 0);
    tileMesh.drawInstanced(tilecount);
    glUniform1i(instLoc, 0);

    //draw buildings
    for (auto &b : w->buildings()) {
        if (!b) continue;

        const buildingDef d = defOf(b->type);

        float yaw = -(int)b->dir * 90.0f;

        glm::vec3 pos(b->pos.x, 0.5f + d.height * 0.5f, b->pos.y);
        drawCube(pos, {0.9f, d.height, 0.9f}, yaw, d.col, false);

        drawPorts(b->type, b->pos, b->dir, 1.0f);
    }

    if (hoverValid && selected >= 0) {
        const buildingDef &d = defOf((objectType)selected);
        bool ok = w->canPlace((objectType)selected, hoverCell);

        glm::vec4 col = ok ? glm::vec4(0.2f, 1.0f, 0.3f, 0.5f) : glm::vec4(1.0f, 0.2f, 0.2f, 0.5f);
        drawCube({hoverCell.x, 0.5f + d.height * 0.5f, hoverCell.y},{0.9f, d.height, 0.9f}, -(int)placeDir * 90.0f, col, true);

        drawPorts((objectType)selected, hoverCell, placeDir, 0.8f);
    }

    //draw everything else
    for (auto& [id, obj] : renderables) {
        if (!obj.props.visible) {continue;}

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, obj.props.pos);

        model = glm::rotate(model, glm::radians(obj.props.rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(obj.props.rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(obj.props.rot.z), glm::vec3(0.0f, 0.0f, 1.0f));

        model = glm::scale(model, obj.props.scale);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glUniform4fv(colLoc, 1, glm::value_ptr(obj.props.col));
        glUniform1i(litLoc, obj.props.unlit);

        obj.mesh.draw();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}
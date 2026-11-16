#ifndef FACTORY_ENGINE_H
#define FACTORY_ENGINE_H

#include "tiny_obj_loader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <utility>

#include "util.h"
#include "tiny_obj_loader.h"
#include "world.h"

class engine;
using namespace util;

struct tileInst {
    glm::vec3 pos;
    glm::vec4 col;
};

struct properties {
    glm::vec3 scale = glm::vec3(1);
    glm::vec3 pos = glm::vec3(0, 0, 0);
    glm::vec3 rot = glm::vec3(0, 0, 0);

    glm::vec4 col = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);

    bool unlit = false;
    bool visible = true;
};

struct mesh {
    vector<glm::vec3> vertices;
    vector<unsigned int> indices;
    vector<glm::vec3> normals;
    vector<glm::vec2> texcoords;

    unsigned int VAO, VBO, EBO = 0;
    int indexCount = 0;

    void create() {
        vector<float> data;

        //combine data to a single vector, 3 values for pos, 3 for normal, and 2 for uv
        for (int i = 0; i < vertices.size(); i++) {
            //pos
            data.insert(data.end(), {vertices[i].x, vertices[i].y, vertices[i].z});

            //normals
            //default value incase one doesnt exist
            glm::vec3 n = i < normals.size() ? normals[i] : glm::vec3(0, 1, 0);
            data.insert(data.end(), {n.x, n.y, n.z});

            //uvs
            //default value
            glm::vec2 t = i < texcoords.size() ? texcoords[i] : glm::vec2(0);
            data.insert(data.end(),{t.x, t.y});
        }
        indexCount = (int)indices.size();

        //buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        //vertex attribs
        GLsizei stride = 8 * sizeof(float);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        //unbind vao
        glBindVertexArray(0);
    }

    void draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void destroy() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    static mesh cube() {
        mesh m;
        struct Face {glm::vec3 n, u, v; };

        Face faces[6] = {
            {{ 1,0,0},{0,1,0},{0,0,1}}, {{-1,0,0},{0,0,1},{0,1,0}},
            {{0, 1,0},{0,0,1},{1,0,0}}, {{0,-1,0},{1,0,0},{0,0,1}},
            {{0,0, 1},{1,0,0},{0,1,0}}, {{0,0,-1},{0,1,0},{1,0,0}},
        };

        for (Face& f : faces) {
            unsigned int base = (unsigned int)m.vertices.size();

            m.vertices.push_back(0.5f * (f.n - f.u - f.v));
            m.vertices.push_back(0.5f * (f.n + f.u - f.v));
            m.vertices.push_back(0.5f * (f.n + f.u + f.v));
            m.vertices.push_back(0.5f * (f.n - f.u + f.v));

            for (int i=0; i < 4; i++) {
                m.normals.push_back(f.n);
            }

            m.texcoords.insert(m.texcoords.end(), {{0,0},{1,0},{1,1},{0,1}});
            m.indices.insert(m.indices.end(), {base, base+1, base+2, base, base+2, base+3});
        }

        return m;
    }

    static mesh import(string path) {
        mesh m;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;

        bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str(), NULL);

        if (!success) {
            log(ERRO, ("Failed to load obj mesh from path '" + path + "', Error: " + err).c_str());
            return m;
        }

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                glm::vec3 vertex_pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                glm::vec3 vertex_normal = {};
                glm::vec2 vertex_uv = {};

                if (index.normal_index >= 0) {
                    vertex_normal.x = attrib.normals[3 * index.normal_index + 0];
                    vertex_normal.y = attrib.normals[3 * index.normal_index + 1];
                    vertex_normal.z = attrib.normals[3 * index.normal_index + 2];
                } else {
                    log(WARN, ("Mesh '" + path + "' does not have normal vertex attributes").c_str());
                }

                if (index.texcoord_index >= 0) {
                    vertex_uv.x = attrib.texcoords[2 * index.texcoord_index + 0];
                    vertex_uv.y = attrib.texcoords[2 * index.texcoord_index + 1];
                } else {
                    log(WARN, ("Mesh '" + path + "' does not have UV vertex attributes attributes").c_str());
                }

                m.vertices.push_back(vertex_pos);
                m.normals.push_back(vertex_normal);
                m.texcoords.push_back(vertex_uv);
                m.indices.push_back((unsigned int)m.indices.size());
            }
        }

        return m;
    }

    void drawInstanced(int count) {
        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, count);
        glBindVertexArray(0);
    }
};

struct object {
    object(string path) {
        this->mesh = mesh::import(std::move(path));
        this->props = properties();
    }

    object() {
        this->mesh = mesh::cube();
        this->props = properties();
    }

    void create(uint64_t id) {
        this->mesh.create();
        this->id = id;
    }

    mesh mesh;
    properties props;
    uint64_t id;
};

class engine {
public:
    engine() {
        int res = init();

        if (res != 0) {
            glfwTerminate();
            log(ERRO, "INIT FAILED");
        }
    }

    GLFWwindow& getWindow(){return *window;}

    void close();

    void update();
    void render();

    void drawUI();

    void drawPorts(objectType t, glm::ivec2 pos, direction dir, float alpha);

    uint64_t genHandle() {
        handle_ctr++;
        return handle_ctr;
    }

    double startup = 0;
    double lastframe = 0;
    double dt = 0;
    float cameraSpeed = 5.0f;
    int selected = -1;
    bool hoverValid = false;

    glm::ivec2 winSize = {800, 600};

    glm::vec3 camPos = {3, 10, 3};
    glm::vec3 camTarget = {0, 0, 0};
    glm::vec3 cameraTargetOffset = {-1, -4, -1};
    glm::ivec2 worldSize = {100, 100};

    direction placeDir = NORTH;
    glm::ivec2 hoverCell = {0,0};

    world *w = NULL;

    std::unordered_map<uint64_t, object> renderables;

    mesh tileMesh;
    unsigned int tileInstVBO = 0;
    int tilecount;
private:
    int init();

    uint64_t handle_ctr = 0;

    GLFWwindow* window;

    void handlePlacement();
    void drawCube(glm::vec3 pos, glm::vec3 scale, float yawDeg, glm::vec4 col, bool unlit);

    //uniforms
    unsigned int viewLoc = 0;
    unsigned int projLoc = 0;
    unsigned int modelLoc = 0;
    unsigned int camPosLoc = 0;
    unsigned int colLoc = 0;
    unsigned int litLoc = 0;
    unsigned int instLoc = 0;
    unsigned int shader_program;

    float FOV = 70.0f;
    float statsBottom = 0.0f;

    int lastframec = 0;
    int frames = 0;

    double lastframeT = 0;
    bool hasInspected = false;

    bool prevR = false;
    bool prevH = false;
    bool showTutorial = true;

    std::vector<bool> goalDone;

    double incomeTimer = 0.0;
    int incomeLast = 0;
    int incomePerSec = 0;

    glm::mat4 projM = glm::mat4(1), viewM = glm::mat4(1);
    glm::ivec2 inspectedCell = {0,0};
};

#endif
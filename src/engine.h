#ifndef FACTORY_ENGINE_H
#define FACTORY_ENGINE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "util.h"

using namespace util;

struct mesh {
    vector<glm::vec3> vertices;
    vector<unsigned int> indices;
    vector<glm::vec3> normals;
    vector<glm::vec2> texcoords;

    unsigned int VAO, VBO, EBO;
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

    double startup = 0;
    double lastframe = 0;

    glm::ivec2 winSize = {800, 600};
private:
    int init();

    GLFWwindow* window;

    unsigned int shader_program;

    //uniforms
    unsigned int viewLoc;
    unsigned int projLoc;
    unsigned int modelLoc;
    unsigned int camPosLoc;

    glm::vec3 camPos = {3, 2, 3};
    glm::vec3 camTarget = {0, 0, 0};

    float FOV = 70.0f;

    mesh cube;
};

#endif
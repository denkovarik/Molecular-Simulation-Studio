// src/classes/Renderer.hpp

#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <glm/gtx/string_cast.hpp>  // for glm::to_string

struct Config;  // forward declare

class Simulation; // forward declare 

class Renderer 
{
public:
    explicit Renderer(const Config& cfg);
    ~Renderer();

    // Delete copy, allow move wanted
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void render(const Simulation& sim);
    GLFWwindow* getWindow() const { return m_window; }

    // Optional helpers
    void setView(const glm::mat4& view)   { m_view = view; }
    void setProjection(const glm::mat4& proj) { m_proj = proj; }
                    
    void drawBond(const glm::vec3& a, const glm::vec3& b);              

private:
    GLuint m_cylinderVAO = 0;
    GLuint m_cylinderVBO = 0;
    GLuint m_cylinderEBO = 0;
    GLuint m_cylinderIndexCount = 0;
    
    void initShaders();
    void initUnitSphere(int sectors = 20, int stacks = 20);

    GLFWwindow* m_window = nullptr;

    GLuint m_shaderProgram = 0;
    GLuint m_VAO = 0, m_VBO = 0, m_EBO = 0;
    unsigned int m_indexCount = 0;

    glm::mat4 m_view;
    glm::mat4 m_proj;

    int m_width, m_height;
};

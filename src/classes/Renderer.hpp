#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

struct Config;  // forward declare

class Simulation; // forward declare (we'll make this soon)

class Renderer {
public:
    explicit Renderer(const Config& cfg);
    ~Renderer();

    // Delete copy, allow move if you want (optional)
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void render(const Simulation& sim);
    GLFWwindow* getWindow() const { return m_window; }

    // Optional helpers
    void setView(const glm::mat4& view)   { m_view = view; }
    void setProjection(const glm::mat4& proj) { m_proj = proj; }

private:
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

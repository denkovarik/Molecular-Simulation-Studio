// src/classes/Renderer.cpp (updated loop to use sim.getParticles())
#include "Renderer.hpp"
#include "config.hpp"
#include "Simulation.hpp"
#include "Particle.hpp"  // needed for generateSphere signature
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <stdexcept>
// ─────────────────────────────────────────────────────────────────────────────
// Shaders
// ──────────────────────────────────────────────────────────────────────────────
const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
out vec3 fragPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    fragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(fragPos, 1.0);
}
)";
const char* fragmentSrc = R"(
#version 330 core
out vec4 FragColor;
in vec3 fragPos;
uniform vec3 sphereCenter;
uniform float radius;
uniform vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
void main() {
    vec3 N = normalize(fragPos - sphereCenter);
    float diff = max(dot(N, lightDir), 0.0);
    vec3 baseColor = vec3(1.0, 0.25, 0.15);
    vec3 color = baseColor * (0.3 + 0.7 * diff);
    // Latitude / longitude contour lines
    float phi = acos(N.y);
    float theta = atan(N.z, N.x);
    float lat = fract(phi * 8.0);
    float lon = fract((theta + 3.14159265) / 0.4);
    float contour = 1.0 - 0.35 * (step(0.98, lat) + step(0.98, lon));
    color *= contour;
    FragColor = vec4(color, 1.0);
}
)";
// ─────────────────────────────────────────────────────────────────────────────
// Helper: compile & link shaders
// ─────────────────────────────────────────────────────────────────────────────
static GLuint compileShader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::string typeName = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
        throw std::runtime_error("Shader compilation failed (" + typeName + "):\n" + infoLog);
    }
    return shader;
}
static GLuint createProgram(const char* vSrc, const char* fSrc) {
    GLuint vs = compileShader(vSrc, GL_VERTEX_SHADER);
    GLuint fs = compileShader(fSrc, GL_FRAGMENT_SHADER);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(prog, 512, nullptr, infoLog);
        throw std::runtime_error("Program linking failed:\n" + std::string(infoLog));
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}
// ─────────────────────────────────────────────────────────────────────────────
// generateSphere (unit sphere → radius = 1.0)
// ─────────────────────────────────────────────────────────────────────────────
static void generateSphere(float radius, int sectors, int stacks,
                           std::vector<float>& vertices,
                           std::vector<unsigned int>& indices) {
    vertices.clear();
    indices.clear();
    for (int i = 0; i <= stacks; ++i) {
        float phi = glm::pi<float>() * i / stacks;
        for (int j = 0; j <= sectors; ++j) {
            float theta = 2.0f * glm::pi<float>() * j / sectors;
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);
            vertices.insert(vertices.end(), {x, y, z});
        }
    }
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < sectors; ++j) {
            unsigned int first = i * (sectors + 1) + j;
            unsigned int second = first + sectors + 1;
            indices.insert(indices.end(), {first, second, first + 1});
            indices.insert(indices.end(), {second, second + 1, first + 1});
        }
    }
}
// ─────────────────────────────────────────────────────────────────────────────
// Renderer implementation
// ─────────────────────────────────────────────────────────────────────────────
Renderer::Renderer(const Config& cfg)
    : m_width(cfg.windowWidth), m_height(cfg.windowHeight)
{
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    m_window = glfwCreateWindow(m_width, m_height, cfg.windowTitle.c_str(), nullptr, nullptr);
    if (!m_window)
        throw std::runtime_error("Failed to create GLFW window");
    glfwMakeContextCurrent(m_window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("Failed to initialize GLEW");
    glEnable(GL_DEPTH_TEST);
    m_shaderProgram = createProgram(vertexSrc, fragmentSrc);
    initUnitSphere(cfg.sphereSectors, cfg.sphereStacks);
    m_view = glm::lookAt(glm::vec3(0.0f, 0.0f, 8.0f),
                         glm::vec3(0.0f, 0.0f, 0.0f),
                         glm::vec3(0.0f, 1.0f, 0.0f));
    m_proj = glm::perspective(glm::radians(45.0f),
                              static_cast<float>(m_width) / m_height,
                              0.1f, 100.0f);
}
Renderer::~Renderer() {
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_EBO) glDeleteBuffers(1, &m_EBO);
    if (m_window) glfwDestroyWindow(m_window);
    glfwTerminate();
}
void Renderer::initUnitSphere(int sectors, int stacks) {
    std::vector<float> verts;
    std::vector<unsigned int> inds;
    generateSphere(1.0f, sectors, stacks, verts, inds);
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned int), inds.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    m_indexCount = static_cast<unsigned int>(inds.size());
}
void Renderer::render(const Simulation& sim) {
    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_shaderProgram);
    glBindVertexArray(m_VAO);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(m_view));
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(m_proj));
    for (const Particle& p : sim.getParticles()) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), p.position);
        model = glm::scale(model, glm::vec3(p.radius));
        glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(glGetUniformLocation(m_shaderProgram, "sphereCenter"), 1, glm::value_ptr(p.position));
        glUniform1f(glGetUniformLocation(m_shaderProgram, "radius"), p.radius);
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    }
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

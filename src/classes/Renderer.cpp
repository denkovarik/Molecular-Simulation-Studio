// src/classes/Renderer.cpp
#include "Renderer.hpp"
#include "config.hpp"
#include "Simulation.hpp"
#include "Particle.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp> // <-- This gives glm::to_string()
#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
// Shaders
// ─────────────────────────────────────────────────────────────────────────────
const char* vertexSrc = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    out vec3 FragPos;
    out vec3 Normal;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main() {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
)";

const char* fragmentSrc = R"(
    #version 330 core
    out vec4 FragColor;
    in vec3 FragPos;
    in vec3 Normal;
    uniform vec3 lightPos;
    uniform vec3 viewPos;
    uniform vec3 lightColor;
    uniform vec3 objectColor;
    uniform float ambientStrength = 0.1;
    uniform float specularStrength = 0.5;
    uniform int shininess = 32;
    uniform vec3 emission = vec3(0.0);
    void main() {
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        // ambient
        vec3 ambient = ambientStrength * lightColor;
        // diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        // specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = specularStrength * spec * lightColor;
        // result
        vec3 result = (ambient + diffuse + specular) * objectColor + emission;
        FragColor = vec4(result, 1.0);
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
// generateSphere & generateCylinder
// ─────────────────────────────────────────────────────────────────────────────
static void generateSphere(float radius, int sectors, int stacks,
                         std::vector<float>& vertices,
                         std::vector<unsigned int>& indices) {
    vertices.clear(); indices.clear();
    for (int i = 0; i <= stacks; ++i) {
        float phi = glm::pi<float>() * i / stacks;
        for (int j = 0; j <= sectors; ++j) {
            float theta = 2.0f * glm::pi<float>() * j / sectors;
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);
            vertices.insert(vertices.end(), {x, y, z, x / radius, y / radius, z / radius});
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

static void generateCylinder(std::vector<float>& vertices,
                           std::vector<unsigned int>& indices,
                           float radius = 0.06f,
                           int sectors = 20) {
    vertices.clear(); indices.clear();
    for (int i = 0; i < 2; ++i) {
        float z = (i == 0) ? -0.5f : 0.5f;
        for (int j = 0; j <= sectors; ++j) {
            float angle = 2.0f * glm::pi<float>() * j / sectors;
            float x = radius * cosf(angle);
            float y = radius * sinf(angle);
            vertices.insert(vertices.end(), {x, y, z, x / radius, y / radius, 0.0f});
        }
    }
    for (int j = 0; j < sectors; ++j) {
        unsigned int a = j;
        unsigned int b = j + 1;
        unsigned int c = j + sectors + 1;
        unsigned int d = j + sectors + 2;
        indices.insert(indices.end(), {a, b, d, d, c, a});
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Renderer implementation
// ─────────────────────────────────────────────────────────────────────────────
Renderer::Renderer(const Config& cfg)
    : m_width(cfg.windowWidth), m_height(cfg.windowHeight)
{
    if (!glfwInit()) throw std::runtime_error("Failed to initialize GLFW");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    m_window = glfwCreateWindow(m_width, m_height, cfg.windowTitle.c_str(), nullptr, nullptr);
    if (!m_window) throw std::runtime_error("Failed to create GLFW window");
    glfwMakeContextCurrent(m_window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) throw std::runtime_error("Failed to initialize GLEW");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_shaderProgram = createProgram(vertexSrc, fragmentSrc);
    // Atom sphere
    initUnitSphere(cfg.sphereSectors, cfg.sphereStacks);
    // Bond cylinder
    std::vector<float> cylVerts;
    std::vector<unsigned int> cylInds;
    generateCylinder(cylVerts, cylInds, 0.06f, 20);
    glGenVertexArrays(1, &m_cylinderVAO);
    glGenBuffers(1, &m_cylinderVBO);
    glGenBuffers(1, &m_cylinderEBO);
    glBindVertexArray(m_cylinderVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_cylinderVBO);
    glBufferData(GL_ARRAY_BUFFER, cylVerts.size() * sizeof(float), cylVerts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cylinderEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylInds.size() * sizeof(unsigned int), cylInds.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    m_cylinderIndexCount = static_cast<GLuint>(cylInds.size());
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
    if (m_cylinderVAO) glDeleteVertexArrays(1, &m_cylinderVAO);
    if (m_cylinderVBO) glDeleteBuffers(1, &m_cylinderVBO);
    if (m_cylinderEBO) glDeleteBuffers(1, &m_cylinderEBO);
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    m_indexCount = static_cast<unsigned int>(inds.size());
}

void Renderer::drawBond(const glm::vec3& a, const glm::vec3& b) {
    glm::vec3 dir = b - a;
    float len = glm::length(dir);
    if (len < 1e-4f) return;
    glm::vec3 axis = glm::normalize(dir);
    glm::vec3 up = glm::abs(axis.y) < 0.9f ? glm::vec3(0,1,0) : glm::vec3(1,0,0);
    glm::vec3 right = glm::normalize(glm::cross(axis, up));
    up = glm::cross(right, axis);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, a + dir * 0.5f);
    model = model * glm::mat4(glm::mat3(right, up, -axis));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, len));
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(glGetUniformLocation(m_shaderProgram, "objectColor"), 0.0f, 0.5f, 0.5f);
    glUniform3f(glGetUniformLocation(m_shaderProgram, "emission"), 0.3f, 0.9f, 0.9f);
    glBindVertexArray(m_cylinderVAO);
    glDrawElements(GL_TRIANGLES, m_cylinderIndexCount, GL_UNSIGNED_INT, 0);
}

void Renderer::render(const Simulation& sim) {
    glUniform3f(glGetUniformLocation(m_shaderProgram, "objectColor"), 1.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(m_view));
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(m_proj));
    glUniform3f(glGetUniformLocation(m_shaderProgram, "lightPos"), 0.0f, 0.0f, 10.0f);
    glUniform3f(glGetUniformLocation(m_shaderProgram, "viewPos"), 0.0f, 0.0f, 8.0f);
    glUniform3f(glGetUniformLocation(m_shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
    // Draw atoms (white)
    glBindVertexArray(m_VAO);
    for (const Particle& p : sim.container.particles) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), p.position);
        model = glm::scale(model, glm::vec3(p.radius));
        glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(glGetUniformLocation(m_shaderProgram, "objectColor"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(m_shaderProgram, "emission"), 0.0f, 0.0f, 0.0f);
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    }
    // Draw bonds
    if (sim.config.enable_chemistry) {
        const auto& particles = sim.container.particles;
        for (size_t i = 0; i < particles.size(); ++i) {
            for (size_t j = i + 1; j < particles.size(); ++j) {
                float d = glm::length(particles[j].position - particles[i].position);
                if (d < 1.5f) {
                    drawBond(particles[i].position, particles[j].position);
                }
            }
        }
    }
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

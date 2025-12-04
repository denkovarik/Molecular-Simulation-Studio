// Usage:
// g++ -o render.exe render.cpp src/classes/Particle.cpp src/classes/Container.cpp -I/usr/include -I/usr/include/GL -L/usr/lib -lglfw -lGL -lGLEW -lGLU -lm -lX11 -lXxf86vm -lXrandr -lpthread -ldl -lXinerama -lXcursor
// ./render.exe

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include "./src/classes/Container.hpp"

// Vertex Shader
const char* vertexShaderSource = R"(
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

// Fragment Shader with horizontal and vertical contours + lighting
const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    in vec3 fragPos;
    uniform vec3 sphereCenter;
    uniform float radius;
    uniform float contourSpacing; // For horizontal (latitude) lines
    uniform float verticalSpacing; // For vertical (longitude) lines
    uniform vec3 lightDir;
    void main() {
        vec3 localPos = fragPos - sphereCenter;
        vec3 norm = normalize(localPos);
        vec3 baseColor = vec3(1.0, 0.0, 0.0); // Red
        // Diffuse lighting
        vec3 lightNorm = normalize(lightDir);
        float diff = max(dot(norm, lightNorm), 0.0);
        vec3 color = baseColor * (0.2 + 0.8 * diff); // Ambient + diffuse
        // Horizontal contours (latitude, based on polar angle phi)
        float phi = acos(localPos.y / radius);
        float h_scaled = phi / (3.14159265359 * contourSpacing);
        float h_f = fract(h_scaled);
        float h_df = fwidth(h_scaled);
        float h_g = smoothstep(h_df * 1.0, h_df * 2.0, h_f);
        // Vertical contours (longitude, based on azimuthal angle)
        float angle = atan(localPos.z, localPos.x);
        float v_normalized = angle / (2.0 * 3.14159265359) + 0.5; // 0 to 1
        float v_scaled = v_normalized / verticalSpacing;
        float v_f = fract(v_scaled);
        float v_df = fwidth(v_scaled);
        float v_g = smoothstep(v_df * 1.0, v_df * 2.0, v_f);
        // Combine contours (darken where lines are)
        float c = h_g * v_g;
        color *= c;
        FragColor = vec4(color, 1.0);
    }
)";

GLuint compileAndLinkShaders(const char* vertexSrc, const char* fragmentSrc) {
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vertexSrc, NULL);
    glCompileShader(vShader);
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fragmentSrc, NULL);
    glCompileShader(fShader);
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vShader);
    glAttachShader(shaderProgram, fShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vShader);
    glDeleteShader(fShader);
    return shaderProgram;
}

void generateSphere(float radius, int sectors, int stacks, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
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

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Bouncing Balls", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    glewInit();
    GLuint shaderProgram = compileAndLinkShaders(vertexShaderSource, fragmentShaderSource);
    Container c1 = Container(2.23f, -2.23f, 1.65f, -1.65f, 2.0f, -5.0f);
    int numParticles = 5000;
    // Seed with random device
    std::random_device rd;
    std::mt19937 gen(rd());
    // Generate Particles
    while(c1.particles.size() < numParticles) {
        // Define the distribution
        std::uniform_real_distribution<float> x_pos_distrib(c1.leftWallX + 0.2, c1.rightWallX - 0.2);
        std::uniform_real_distribution<float> y_pos_distrib(c1.floorY + 0.2, c1.ceilingY - 0.2);
        std::uniform_real_distribution<float> z_pos_distrib(c1.backWallZ + 0.2, c1.frontWallZ - 0.2);
        std::uniform_real_distribution<float> vel_distrib(-0.1, 0.1);
        bool done = false;
        while(!done) {
            // Generate Particle
            glm::vec3 position = glm::vec3(
                x_pos_distrib(gen), y_pos_distrib(gen), z_pos_distrib(gen)
            );
            glm::vec3 velocity = glm::vec3(vel_distrib(gen), vel_distrib(gen), vel_distrib(gen));
            Particle newParticle = Particle(1.0f, 0.1f, position, velocity);
            bool collision = false;
            for(int j = 0; j < c1.particles.size() && !collision; j++) {
                if(c1.particlesCollide(newParticle, c1.particles[j])) {
                    collision = true;
                }
            }
            if(!collision) {
                c1.particles.push_back(newParticle);
                done = true;
            }
        }
    }
    
    // Generate a single UNIT SPHERE (radius=1.0f) - shared for all particles
    std::vector<float> unitSphereVertices;
    std::vector<unsigned int> unitSphereIndices;
    // Higher resolution for smoother spheres (adjust as needed)
    generateSphere(1.0f, 20, 20, unitSphereVertices, unitSphereIndices); 
    // Create shared buffers
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER, unitSphereVertices.size() * sizeof(float), 
        unitSphereVertices.data(), GL_STATIC_DRAW
    );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, unitSphereIndices.size() * sizeof(unsigned int), 
        unitSphereIndices.data(), GL_STATIC_DRAW
    );
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glEnable(GL_DEPTH_TEST); // Enable once, outside any loop
    float lastTimeFrame = glfwGetTime();
    int count = 0;
    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTimeFrame;
        lastTimeFrame = currentTime;
        // Simulation: Run ONCE per frame (before rendering)
        c1.assignParticles2Grid();
        c1.resolveParticleCollisions();
        c1.checkWallCollisions();
        for(auto& p : c1.particles) {
            p.updatePosition();  
        }
        // Rendering
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        // Bind shared VAO once per frame
        glBindVertexArray(VAO);
        // Shared view/projection (compute once outside loop)
        glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 4), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.0f);
        glUniformMatrix4fv(
            glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view)
        );
        glUniformMatrix4fv(
            glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(proj)
        );
        for(int i = 0; i < c1.particles.size(); i++) {
            // Camera/View/Projection (model per-particle)
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, c1.particles[i].position);
            model = glm::scale(model, glm::vec3(c1.particles[i].radius)); 
            glUniformMatrix4fv(glGetUniformLocation(
                shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model)
            );
            // Contour settings
            glUniform3fv(
                glGetUniformLocation(shaderProgram, "sphereCenter"), 1, 
                glm::value_ptr(c1.particles[i].position)
            );
            glUniform1f(glGetUniformLocation(shaderProgram, "radius"), c1.particles[i].radius);
            glUniform1f(glGetUniformLocation(shaderProgram, "contourSpacing"), 0.1f);
            glUniform1f(glGetUniformLocation(shaderProgram, "verticalSpacing"), 0.05f);
            glUniform3f(glGetUniformLocation(shaderProgram, "lightDir"), 1.0f, 1.0f, 1.0f);
            glDrawElements(GL_TRIANGLES, unitSphereIndices.size(), GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0); // Unbind VAO
        glfwSwapBuffers(window);
        glfwPollEvents();
        int waitTime = 20 - deltaTime * 1000;
        if(waitTime > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
        }
    }
    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

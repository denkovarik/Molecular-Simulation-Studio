// Usage:
//  g++ -o render render.cpp src/classes/Particle.cpp src/classes/Container.cpp -I/usr/include -I/usr/include/GL -L/usr/lib -lglfw -lGL -lGLEW -lGLU -lm -lX11 -lXxf86vm -lXrandr -lpthread -ldl -lXinerama -lXcursor
//  ./render


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <iostream>
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

// Fragment Shader with horizontal and vertical contours
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
in vec3 fragPos;

void main() {
    vec3 baseColor = vec3(1.0, 0.0, 0.0); // Red
    vec3 color = baseColor; // No lighting calculation, just use the base color

    // Vertical contour lines using azimuthal angle
    float angle = atan(fragPos.z, fragPos.x); // [-pi, pi]
    float wrappedAngle = angle / (2.0 * 3.14159265359); // normalize to [-0.5, 0.5]
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "Bouncing Ball", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    glewInit();

    GLuint shaderProgram = compileAndLinkShaders(vertexShaderSource, fragmentShaderSource);

    Container c1 = Container();
    c1.rightWallX = 2.23f;
    c1.leftWallX = -2.23f;
    c1.ceilingY = 1.65f;
    c1.floorY = -1.65f;
    c1.frontWallZ = 2.0f;
    c1.backWallZ = -5.0f;
    
    c1.particles.push_back(Particle(1.0f, 0.1f, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)));  
    
    glm::vec3 force = glm::vec3(0.01f, 0.01f, 0.01f); // force

    generateSphere(c1.particles[0].radius, 10, 10, c1.particles[0].vertices, c1.particles[0].indices);

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, c1.particles[0].vertices.size() * sizeof(float), c1.particles[0].vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, c1.particles[0].indices.size() * sizeof(unsigned int), c1.particles[0].indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);
    
    float lastTimeFrame = glfwGetTime();  
    
    int count = 0;

    while (!glfwWindowShouldClose(window)) {        
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        
               
        float deltaTime = glfwGetTime() - lastTimeFrame;
        lastTimeFrame = glfwGetTime();
    
        // Update velocity and positions
        if(count < 5) {
            count += 1;
            c1.particles[0].applyForce(force);
        }
        
        c1.checkWallCollisions();
        c1.particles[0].updatePosition();
        
        // Camera/View/Projection
        glm::mat4 model = glm::mat4(1.0f);
        //glm::mat4 model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime() * 0.5f, glm::vec3(0, 1, 0));
        model = glm::translate(model, c1.particles[0].position);
        glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 4), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

        // Contour settings
        glUniform1f(glGetUniformLocation(shaderProgram, "contourSpacing"), 0.1f);
        glUniform1f(glGetUniformLocation(shaderProgram, "verticalSpacing"), 0.05f);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightDir"), 1.0f, 1.0f, 1.0f);

        //glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, c1.particles[0].indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// main_h2_simple.cpp
// g++ -std=c++17 -O2 -I src -I src/classes src/classes/*.cpp src/physics/*.cpp main_h2_simple.cpp -lglfw -lGLEW -lGL -o h2_simple
#include "src/config.hpp"
#include "src/classes/Simulation.hpp"
#include "src/classes/Renderer.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <GLFW/glfw3.h>
int main() {
    Config config;
    config.numParticles = 50;
    //config.spawnRandomParticles = false;
    config.particleRadius = 0.1f;
    config.particleMass = 1.00784f;
    config.windowWidth = 1000;
    config.windowHeight = 800;
    config.windowTitle = "H + H Collision";
    config.containerMinX = config.containerMinY = config.containerMinZ = -5.0f;
    config.containerMaxX = config.containerMaxY = config.containerMaxZ = 5.0f;
    
    config.enable_chemistry = true;
    config.enable_bonds = true;
    config.bond_form_dist = 1.0f;
    config.bond_a = 2.0f;
    config.bond_De = 4.52f;
    config.enable_damping = true;
    config.wall_collision_dampening = 0.8;
    
    try {
        Renderer renderer(config);
        glfwSwapInterval(1); // Enable VSync for smooth frames
        Simulation simulation(config);
        //auto& particles = simulation.container.particles;
        //particles.emplace_back(1.0f, 0.1f, glm::vec3(-5.0f, 0.0f, 0.0f), glm::vec3(6.0f, 0.0f, 0.0f));
        //particles.emplace_back(1.0f, 0.1f, glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(-3.0f, 0.0f, 0.0f));
        //particles[0].elementSymbol = "H";
        //particles[1].elementSymbol = "H";
        float lastTime = static_cast<float>(glfwGetTime());
        std::cout << "H + H collision running — watch them hit!\n";
        while (!glfwWindowShouldClose(renderer.getWindow())) {
            float currentTime = static_cast<float>(glfwGetTime());
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;
            simulation.update(deltaTime);
            renderer.render(simulation);
            std::this_thread::sleep_for(std::chrono::milliseconds(8)); // ~60 FPS cap
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}

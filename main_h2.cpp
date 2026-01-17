// main_h2.cpp

// g++ -std=c++17 -O2 -I src -I src/classes src/classes/*.cpp main_h2.cpp -lglfw -lGLEW -lGL -o h2_sim

#include "src/config.hpp"
#include "src/classes/Simulation.hpp"
#include "src/classes/Renderer.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <GLFW/glfw3.h>
int main() {
    Config config;
    config.numParticles = 2;
    config.particleRadius = 0.15f; // ← THIS IS CRITICAL
    config.particleMass = 1.0f;
    config.enable_chemistry = true;
    config.lj_epsilon = 50.0f; // Reduced for less extreme bonding
    config.lj_sigma = 0.74f; // H-H equilibrium distance
    config.windowWidth = 1000;
    config.windowHeight = 800;
    config.windowTitle = "H₂ Molecule — Glowing Bond!";
    config.containerMinX = config.containerMinY = config.containerMinZ = -6.0f;
    config.containerMaxX = config.containerMaxY = config.containerMaxZ = 6.0f;
    try {
        Renderer renderer(config);
        Simulation simulation(config);
        auto& particles = simulation.container.particles;
        // CRITICAL: Start atoms farther apart with small incoming speeds
        particles[0].position = glm::vec3(-1.0f, 0.0f, 0.0f);
        particles[0].velocity = glm::vec3( 5.1f, 0.0f, 0.0f);
        particles[1].position = glm::vec3( 1.0f, 0.0f, 0.0f);
        particles[1].velocity = glm::vec3(-5.1f, 0.0f, 0.0f);
        const float fixed_dt = 0.0001f;
        float accumulator = 0.0f;
        auto lastTime = std::chrono::high_resolution_clock::now();
        std::cout << "H₂ simulation running — LOOK FOR THE GLOWING CYAN BOND!\n";
        while (!glfwWindowShouldClose(renderer.getWindow())) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
            accumulator += frameTime;
            while (accumulator >= fixed_dt) {
                simulation.update(fixed_dt);
                accumulator -= fixed_dt;
            }
            renderer.render(simulation);
            glfwPollEvents();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}

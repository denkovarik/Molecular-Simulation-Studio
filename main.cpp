// main.cpp
/*
  Molecular Dynamics / Bouncing Balls Simulator
  Now fully modular: Renderer + Simulation + Config
  
*/

#include "src/config.hpp"
#include "src/classes/Simulation.hpp"
#include "src/classes/Renderer.hpp"

#include <iostream>
#include <chrono>
#include <thread>

// Optional: simple command-line config override
Config parseConfig(int argc, char** argv) {
    Config cfg;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--particles" && i+1 < argc) cfg.numParticles = std::stoi(argv[++i]);
        else if (arg == "--radius" && i+1 < argc) cfg.particleRadius = std::stof(argv[++i]);
        else if (arg == "--width" && i+1 < argc) cfg.windowWidth = std::stoi(argv[++i]);
        else if (arg == "--height" && i+1 < argc) cfg.windowHeight = std::stoi(argv[++i]);
        else if (arg == "--title" && i+1 < argc) cfg.windowTitle = argv[++i];
        // Add more as needed: --mass, --velocity, etc.
    }
    return cfg;
}

int main(int argc, char** argv) {
    Config config = parseConfig(argc, argv);

    try {
        // Create renderer (owns window + OpenGL)
        Renderer renderer(config);

        // Create simulation (owns physics + particles)
        Simulation simulation(config);

        float lastTime = static_cast<float>(glfwGetTime());

        std::cout << "Starting simulation with " << config.numParticles
                  << " particles (radius = " << config.particleRadius << ")\n";
        std::cout << "Press ESC or close window to exit.\n";

        // Main loop
        while (!glfwWindowShouldClose(renderer.getWindow())) {  // assuming you expose getWindow()
            float currentTime = static_cast<float>(glfwGetTime());
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            // Update physics
            simulation.update(deltaTime);

            // Render everything
            renderer.render(simulation);

            // Optional: cap to ~60 FPS for smoother feel
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}

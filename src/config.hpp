#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

struct Config {
    // Window
    int windowWidth = 1000;
    int windowHeight = 1000;
    std::string windowTitle = "Bouncing Balls";

    // Container (room)
    float containerMaxX = 10.0f;
    float containerMinX = -10.0f;
    float containerMaxY = 10.0f;
    float containerMinY = -10.0f;
    float containerMaxZ = 10.0f;
    float containerMinZ = -10.0f;

    // Particles
    int numParticles = 1000;
    float particleMass = 1.0f;
    float particleRadius = 0.1f;
    float velocityRange = 5.0f;  // For random vel_distrib(-range, range)

    // Rendering
    int sphereSectors = 20;
    int sphereStacks = 20;

    // Simulation
    float targetFPS = 50.0f;  // For throttling
    
    bool enable_chemistry = false;  // New: Toggle chemical forces (e.g., LJ)
    float lj_epsilon = 0.5f;       // Depth of potential well (tune for attraction strength)
    float lj_sigma = 0.74f;        // Equilibrium distance for H-H 
    // Add more chemistry params later (e.g., per-atom types)
};

#endif // CONFIG_HPP

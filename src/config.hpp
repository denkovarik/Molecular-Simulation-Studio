#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

struct Config {
    // Window
    int windowWidth = 800;
    int windowHeight = 600;
    std::string windowTitle = "Bouncing Balls";

    // Container (room)
    float containerMaxX = 2.23f;
    float containerMinX = -2.23f;
    float containerMaxY = 1.65f;
    float containerMinY = -1.65f;
    float containerMaxZ = 2.0f;
    float containerMinZ = -5.0f;

    // Particles
    int numParticles = 500;
    float particleMass = 1.0f;
    float particleRadius = 0.1f;
    float velocityRange = 5.0f;  // For random vel_distrib(-range, range)

    // Rendering
    int sphereSectors = 20;
    int sphereStacks = 20;

    // Simulation
    float targetFPS = 50.0f;  // For throttling
};

#endif // CONFIG_HPP

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
    float velocityRange = 5.0f;

    // init/spawn controls (Option B)
    bool spawnRandomParticles = true;          // if false, Simulation() won't auto-populate particles
    int  maxPlacementAttemptsPerParticle = 2000; // prevents infinite loops when packing is impossible

    // Rendering
    int sphereSectors = 20;
    int sphereStacks = 20;

    // Simulation
    float targetFPS = 50.0f;

    // Chemistry (LJ)
    bool enable_chemistry = false;
    float lj_epsilon = 0.5f;
    float lj_sigma = 0.74f;

    // Coulomb (you already added these earlier, keeping here for completeness)
    bool  enable_coulomb = false;
    float coulomb_k = 10.0f;
    float coulomb_softening = 0.1f;
};

#endif // CONFIG_HPP


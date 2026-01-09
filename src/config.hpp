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

    // init/spawn controls
    bool spawnRandomParticles = true;          
    int  maxPlacementAttemptsPerParticle = 2000; 

    // Rendering
    int sphereSectors = 20;
    int sphereStacks = 20;

    // Simulation
    float targetFPS = 50.0f;

    // Chemistry (LJ)
    bool enable_chemistry = false;
    float lj_epsilon = 0.5f;
    float lj_sigma = 0.74f;

    // Coulomb 
    bool  enable_coulomb = false;
    float coulomb_k = 10.0f;
    float coulomb_softening = 0.1f;
    
    bool enable_subatomic = false;
    float harmonic_k = 1.0f;
    float harmonic_eq = 0.53f;
    float electron_mass = 1.0f;  // AU (added)
    float proton_mass = 1836.0f;  // AU (added if needed)
    float activation_barrier = 0.1f;
};

#endif // CONFIG_HPP


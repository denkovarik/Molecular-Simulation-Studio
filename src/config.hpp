// src/config.hpp

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

struct Config 
{
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
    float lj_epsilon = 0.452f;
    float lj_sigma = 0.74f;

    // Coulomb 
    bool  enable_coulomb = false;
    float coulomb_k = 10.0f;
    float coulomb_softening = 0.1f;
    
    bool enable_subatomic = false;
    float harmonic_k = 1.0f;
    float harmonic_eq = 0.53f;
    float electron_mass = 1.0f;  // AU 
    float proton_mass = 1836.0f;  // AU 
    float activation_barrier = 0.1f;
    
    bool enable_damping = true;
    float wall_collision_dampening = 0.99;
    
    // Bonding (Phase 1: united-atom)
    bool enable_bonds = false;

    // Distance at which we allow a bond to form (world units)
    float bond_form_dist = 1.5f;

    // Equilibrium bond length for H2 (world units)
    float bond_r0 = 0.74f;

    // Morse parameters 
    float bond_De = 10.0f;   
    float bond_a  = 8.0f;   

    // Break distance 
    float bond_break_dist = 2.5f;

    // Damping along bond axis (stability)
    float bond_axis_damping = 0.05f;

    // QM activation distance (Å)
    float qm_threshold_dist = 2.0f; 
    
    // --- QM surface driving (H2) ---
    bool enable_qm_surface_h2 = false;
    bool enable_atom_forces = true;
    
    // --- QM bond damping (for settling near equilibrium) ---
    bool enable_qm_bond_damping = false;

    // Damping coefficient for relative radial motion (units: force / velocity)
    float qm_bond_damping_gamma = 0.0f;

    bool enable_dynamic_qm = false;  // Compute QM on-the-fly if no PES
    // For QM params (toy defaults; real from Libint2)
    std::string basis_set = "STO-3G";
    bool use_dft = true;
    
    float bond_energy = 4.52f;  // Default H2 bond energy in eV; make configurable
    
    std::string particleElement = "H";  // Default; set per particle
    int maxValence = 1;  // For H
};

#endif // CONFIG_HPP


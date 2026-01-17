// testing/test_BondFormation.cpp
/*

g++ -std=c++17 -O0 -g \
  -I/usr/local/include -I./src -I./src/classes \
  testing/test_BondFormation.cpp \
  src/classes/Simulation.cpp src/classes/Container.cpp src/classes/Particle.cpp \
  src/classes/Atom.cpp src/classes/SubAtomicParticle.cpp \
  src/physics/QmSurface1D.cpp \
  -o test_BondFormation.exe

./test_BondFormation.exe

*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "../src/config.hpp"
#include "../src/classes/Simulation.hpp"
#include <glm/glm.hpp>

TEST_CASE("H + H approaching within bond_form_dist creates exactly one bond") {
    Config cfg;
    cfg.spawnRandomParticles = false;
    cfg.numParticles = 0;

    // Keep legacy particle mode
    cfg.enable_subatomic = false;

    // Disable other forces that could interfere
    cfg.enable_chemistry = false;   // disables LJ in your current code
    cfg.enable_coulomb = false;

    // Big container so no wall collisions
    cfg.containerMinX = cfg.containerMinY = cfg.containerMinZ = -10.0f;
    cfg.containerMaxX = cfg.containerMaxY = cfg.containerMaxZ =  10.0f;

    // --- new config you will add ---
    cfg.enable_bonds = true;
    cfg.bond_form_dist = 1.5f; // choose your value
    cfg.bond_r0 = 0.74f;

    Simulation sim(cfg);
    sim.container.particles.clear();
    sim.container.particles.reserve(2);

    sim.container.particles.emplace_back(
        /*mass*/ 1.0f, /*radius*/ 0.1f,
        /*pos*/ glm::vec3(-2.0f, 0.0f, 0.0f),
        /*vel*/ glm::vec3(+2.0f, 0.0f, 0.0f)
    );
    sim.container.particles.emplace_back(
        /*mass*/ 1.0f, /*radius*/ 0.1f,
        /*pos*/ glm::vec3(+2.0f, 0.0f, 0.0f),
        /*vel*/ glm::vec3(-2.0f, 0.0f, 0.0f)
    );

    // Step forward until they should cross within bond_form_dist
    const float dt = 0.001f;
    for (int step = 0; step < 5000; ++step) {
        sim.update(dt);
        // Once implemented, you can early-out when bond appears.
        // But for now keep it simple.
    }

    // --- new state you will add to Simulation ---
    REQUIRE(sim.bonds.size() == 1);

    // Should not duplicate bonds if we keep stepping
    for (int step = 0; step < 2000; ++step) sim.update(dt);
    REQUIRE(sim.bonds.size() == 1);
}


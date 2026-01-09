// testing/test_Simulation.cpp
/*
Usage:

g++ -std=c++14 -I/usr/local/include -o test_Simulation.exe \
  testing/test_Simulation.cpp \
  src/classes/Simulation.cpp src/classes/Container.cpp src/classes/Particle.cpp

./test_Simulation.exe

*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "../src/config.hpp"
#include "../src/classes/Simulation.hpp"

#include <glm/glm.hpp>

static void requireVecApprox(const glm::vec3& a, const glm::vec3& b, float eps = 1e-6f) {
    REQUIRE(a.x == Approx(b.x).margin(eps));
    REQUIRE(a.y == Approx(b.y).margin(eps));
    REQUIRE(a.z == Approx(b.z).margin(eps));
}

TEST_CASE("Simulation constructs with requested number of particles") {
    Config cfg;
    cfg.numParticles = 1;

    // Keep it simple / deterministic
    cfg.enable_chemistry = false;
    cfg.particleMass = 1.0f;
    cfg.particleRadius = 0.1f;

    // Set a valid container (avoid uninitialized walls in default Container())
    cfg.containerMinX = cfg.containerMinY = cfg.containerMinZ = -2.0f;
    cfg.containerMaxX = cfg.containerMaxY = cfg.containerMaxZ =  2.0f;

    Simulation sim(cfg);

    REQUIRE(sim.container.particles.size() == 1);
    REQUIRE(sim.container.particles[0].mass == Approx(1.0f));
    REQUIRE(sim.container.particles[0].radius == Approx(0.1f));
}

TEST_CASE("Simulation update moves particles by v*dt when chemistry is disabled") {
    Config cfg;
    cfg.numParticles = 1;
    cfg.enable_chemistry = false;

    cfg.particleMass = 1.0f;
    cfg.particleRadius = 0.1f;

    cfg.containerMinX = cfg.containerMinY = cfg.containerMinZ = -10.0f;
    cfg.containerMaxX = cfg.containerMaxY = cfg.containerMaxZ =  10.0f;

    Simulation sim(cfg);

    // Override random init so the test is deterministic
    sim.container.particles[0].position = glm::vec3(0.0f, 0.0f, 0.0f);
    sim.container.particles[0].velocity = glm::vec3(1.0f, 2.0f, 3.0f);

    float dt = 1.0f;
    sim.update(dt);

    requireVecApprox(sim.container.particles[0].position, glm::vec3(1.0f, 2.0f, 3.0f), 1e-5f);
}


// testing/test_Simulation.cpp
/*
Usage:

g++ -std=c++17 \
  -I/usr/local/include/catch2 \
  -I./src -I./src/classes \
  testing/test_Simulation.cpp \
  src/classes/Simulation.cpp \
  src/classes/Container.cpp \
  src/classes/Atom.cpp \
  src/classes/Particle.cpp \
  src/classes/SubAtomicParticle.cpp \
  -o test_Simulation.exe

./test_Simulation.exe

*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <fstream> 
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

// testing/test_Simulation.cpp (or append to test_Atom.cpp for now)
TEST_CASE("Simulation::checkReactions forms bond for close H atoms with sufficient energy") {
    // Arrange: Config with barrier
    Config cfg;
    cfg.activation_barrier = 0.1f;  // Low for test
    cfg.spawnRandomParticles = false;  // Manual

    Simulation sim(cfg);
    // Add two H atoms close with high relative speed
    sim.atoms.emplace_back("H", 1, 1836.0f, 1.0f, 0.53f, 1.0f, 0.53f, glm::vec3(-0.6f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));  // KE ~0.5*1836*1^2 = high
    sim.atoms.emplace_back("H", 1, 1836.0f, 1.0f, 0.53f, 1.0f, 0.53f, glm::vec3(0.6f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f));

    float initial_dist = glm::length(sim.atoms[1].nucleus.position - sim.atoms[0].nucleus.position);
    glm::vec3 v0_before = sim.atoms[0].nucleus.velocity;
    glm::vec3 v1_before = sim.atoms[1].nucleus.velocity;

    // Act: Call checkReactions
    sim.checkReactions();

    // Assert: Positions adjusted to ~0.74 Å apart, centered
    float final_dist = glm::length(sim.atoms[1].nucleus.position - sim.atoms[0].nucleus.position);
    REQUIRE(final_dist == Approx(0.74f).margin(0.01f));
    REQUIRE(initial_dist > final_dist);  // Closer now

    // Velocities damped
    REQUIRE(glm::length(sim.atoms[0].nucleus.velocity) < glm::length(v0_before));
    REQUIRE(glm::length(sim.atoms[1].nucleus.velocity) < glm::length(v1_before));
}

TEST_CASE("Simulation loads element data from JSON correctly") {
    // Arrange: Create temp JSON file for test
    std::string json_filename = "test_elements.json";
    std::ofstream json_file(json_filename);
    json_file << R"({
      "H": {
        "Z": 1,
        "mass": 1836.0,
        "charge": 1.0,
        "vdw_r": 0.53,
        "harmonic_k": 1.0,
        "harmonic_eq": 0.53,
        "barrier": 0.1
      }
    })";
    json_file.close();

    Config cfg;
    Simulation sim(cfg);

    // Act: Load JSON
    sim.loadElementsFromJSON(json_filename);

    // Assert: Params loaded (assume sim has std::map<std::string, ElementData> elementData;)
    REQUIRE(sim.elementData.size() == 1);
    const auto& h_data = sim.elementData["H"];
    REQUIRE(h_data.Z == 1);
    REQUIRE(h_data.mass == Approx(1836.0f));
    REQUIRE(h_data.charge == Approx(1.0f));
    REQUIRE(h_data.vdw_r == Approx(0.53f));
    REQUIRE(h_data.harmonic_k == Approx(1.0f));
    REQUIRE(h_data.harmonic_eq == Approx(0.53f));
    REQUIRE(h_data.barrier == Approx(0.1f));

    // Cleanup
    std::remove(json_filename.c_str());
}

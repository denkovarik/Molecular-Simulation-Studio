// testing/test_CoulombForce.cpp

/*

g++ -std=c++17 \
  -I/usr/local/include/catch2 \
  -I./src -I./src/classes \
  testing/test_CoulombForce.cpp \
  src/classes/Simulation.cpp \
  src/classes/Container.cpp \
  src/classes/Particle.cpp \
  -o test_CoulombForce.exe

./test_CoulombForce.exe

*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <glm/glm.hpp>
#include <cmath>   
#include <limits>  
#include "../src/physics/Coulomb.hpp"
#include "../src/classes/Simulation.hpp"


TEST_CASE("Coulomb force is equal-and-opposite and repulsive for like charges") {
    const float k = 10.0f;
    const float soft = 0.1f;
    const float tol = 1e-6f;

    glm::vec3 x1(-1.0f, 0.0f, 0.0f);
    glm::vec3 x2(+1.0f, 0.0f, 0.0f);

    float q1 = +1.0f;
    float q2 = +1.0f;

    glm::vec3 r12 = x2 - x1;
    glm::vec3 F1 = coulombForce(r12, q1, q2, k, soft);

    glm::vec3 r21 = x1 - x2;
    glm::vec3 F2 = coulombForce(r21, q2, q1, k, soft);

    REQUIRE(F1.x == Approx(-F2.x).epsilon(tol));
    REQUIRE(F1.y == Approx(-F2.y).epsilon(tol));
    REQUIRE(F1.z == Approx(-F2.z).epsilon(tol));

    REQUIRE(F1.x < 0.0f);
    REQUIRE(F2.x > 0.0f);

    glm::vec3 net = F1 + F2;
    REQUIRE(net.x == Approx(0.0f).margin(tol));
    REQUIRE(net.y == Approx(0.0f).margin(tol));
    REQUIRE(net.z == Approx(0.0f).margin(tol));
}

TEST_CASE("Coulomb: equal-and-opposite and attractive for opposite charges") {
    const float k = 10.0f;
    const float soft = 0.1f;
    const float tol = 1e-6f;

    glm::vec3 x1(-1.0f, 0.0f, 0.0f);
    glm::vec3 x2(+1.0f, 0.0f, 0.0f);

    float q1 = +1.0f;
    float q2 = -1.0f;

    glm::vec3 r12 = x2 - x1;
    glm::vec3 F1  = coulombForce(r12, q1, q2, k, soft);

    glm::vec3 r21 = x1 - x2;
    glm::vec3 F2  = coulombForce(r21, q2, q1, k, soft);

    // Equal and opposite
    REQUIRE(F1.x == Approx(-F2.x).epsilon(tol));
    REQUIRE(F1.y == Approx(-F2.y).epsilon(tol));
    REQUIRE(F1.z == Approx(-F2.z).epsilon(tol));

    // Attraction directions
    REQUIRE(F1.x > 0.0f); // particle at -1 pulled right
    REQUIRE(F2.x < 0.0f); // particle at +1 pulled left

    // Net force ~ 0
    glm::vec3 net = F1 + F2;
    REQUIRE(net.x == Approx(0.0f).margin(tol));
    REQUIRE(net.y == Approx(0.0f).margin(tol));
    REQUIRE(net.z == Approx(0.0f).margin(tol));
}

TEST_CASE("Coulomb: softening prevents singularities and produces finite forces") {
    const float k = 10.0f;
    const float soft = 0.1f;

    glm::vec3 r0(0.0f, 0.0f, 0.0f);
    glm::vec3 F0 = coulombForce(r0, +1.0f, +1.0f, k, soft);

    REQUIRE(std::isfinite(F0.x));
    REQUIRE(std::isfinite(F0.y));
    REQUIRE(std::isfinite(F0.z));
    REQUIRE(F0.x == Approx(0.0f));
    REQUIRE(F0.y == Approx(0.0f));
    REQUIRE(F0.z == Approx(0.0f));

    glm::vec3 r_small(1e-6f, 0.0f, 0.0f);
    glm::vec3 F_small = coulombForce(r_small, +1.0f, +1.0f, k, soft);

    REQUIRE(std::isfinite(F_small.x));
    REQUIRE(std::isfinite(F_small.y));
    REQUIRE(std::isfinite(F_small.z));

    // Monotonic falloff holds when r >> soft
    glm::vec3 r1(0.5f, 0.0f, 0.0f);
    glm::vec3 r2(1.0f, 0.0f, 0.0f);

    glm::vec3 F1 = coulombForce(r1, +1.0f, +1.0f, k, soft);
    glm::vec3 F2 = coulombForce(r2, +1.0f, +1.0f, k, soft);

    REQUIRE(glm::length(F1) > glm::length(F2));
}

// Helper for vec3 approx
static void requireVec3Approx(const glm::vec3& a, const glm::vec3& b, float margin) {
    REQUIRE(a.x == Approx(b.x).margin(margin));
    REQUIRE(a.y == Approx(b.y).margin(margin));
    REQUIRE(a.z == Approx(b.z).margin(margin));
}

TEST_CASE("Simulation: Coulomb pair forces conserve total linear momentum (equal-and-opposite)") {
    // ----------------------------
    // Arrange
    // ----------------------------
    Config cfg;

    // IMPORTANT with option B:
    cfg.spawnRandomParticles = false;
    cfg.numParticles = 0; // constructor returns without placing anything

    cfg.particleRadius = 0.01f;
    cfg.particleMass   = 1.0f;

    // Make sure OTHER forces don't interfere with this test:
    cfg.enable_chemistry = false; // disables LJ + damping in Simulation::update()

    // Big container so there are no wall collisions
    cfg.containerMinX = cfg.containerMinY = cfg.containerMinZ = -10.0f;
    cfg.containerMaxX = cfg.containerMaxY = cfg.containerMaxZ =  10.0f;

    // Coulomb toggles
    cfg.enable_coulomb = true;
    cfg.coulomb_k = 10.0f;
    cfg.coulomb_softening = 0.1f;
    
    std::cout << "Simulation construction" << std::endl;
    Simulation sim(cfg);
    std::cout << "Simulation construction done" << std::endl;

    // Manually populate exactly two particles
    sim.container.particles.clear();
    sim.container.particles.reserve(2);

    sim.container.particles.emplace_back(
        /*mass*/ 1.0f,
        /*radius*/ 0.01f,
        /*pos*/ glm::vec3(-1.0f, 0.0f, 0.0f),
        /*vel*/ glm::vec3(0.0f)
    );
    sim.container.particles.emplace_back(
        /*mass*/ 1.0f,
        /*radius*/ 0.01f,
        /*pos*/ glm::vec3(+1.0f, 0.0f, 0.0f),
        /*vel*/ glm::vec3(0.0f)
    );

    auto& ps = sim.container.particles;
    REQUIRE(ps.size() == 2);

    // Charges (assumes Particle has a float charge field)
    ps[0].charge = +1.0f;
    ps[1].charge = +1.0f;

    const float dt = 1e-3f;

    glm::vec3 p_before =
        ps[0].mass * ps[0].velocity +
        ps[1].mass * ps[1].velocity;

    // ----------------------------
    // Act
    // ----------------------------
    sim.update(dt);

    // ----------------------------
    // Assert
    // ----------------------------
    glm::vec3 p_after =
        ps[0].mass * ps[0].velocity +
        ps[1].mass * ps[1].velocity;

    // Total momentum should be conserved (no external forces)
    requireVec3Approx(p_after, p_before, 1e-6f);

    // Like charges should repel: left particle goes more left, right particle goes more right
    REQUIRE(ps[0].velocity.x < 0.0f);
    REQUIRE(ps[1].velocity.x > 0.0f);

    // Symmetry: speeds should match (equal masses, symmetric setup)
    REQUIRE(std::abs(ps[0].velocity.x) ==
            Approx(std::abs(ps[1].velocity.x)).margin(1e-6f));
}





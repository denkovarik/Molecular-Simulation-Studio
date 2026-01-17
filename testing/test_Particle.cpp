// testing/test_Particle.cpp

/* 
Usage:

g++ -std=c++14 -I/usr/local/include -o test_Particle.exe \
    testing/test_Particle.cpp src/classes/Particle.cpp src/classes/Container.cpp

./test_Particle.exe

*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "../src/classes/Particle.hpp"
#include <iostream>

static void requireVecApprox(const glm::vec3& a, const glm::vec3& b, float eps = 1e-6f) {
    REQUIRE(a.x == Approx(b.x).margin(eps));
    REQUIRE(a.y == Approx(b.y).margin(eps));
    REQUIRE(a.z == Approx(b.z).margin(eps));
}

TEST_CASE("Particle constructor sets position/velocity") {
    Particle p1(glm::vec3(0.0f), glm::vec3(0.0f));
    requireVecApprox(p1.position, glm::vec3(0.0f));
    requireVecApprox(p1.velocity, glm::vec3(0.0f));
}

TEST_CASE("Particle constructor sets mass/radius") {
    Particle p2(2.0f, 0.5f, glm::vec3(0.0f), glm::vec3(0.0f));
    REQUIRE(p2.mass == Approx(2.0f));
    REQUIRE(p2.radius == Approx(0.5f));
}

TEST_CASE("applyImpulse changes velocity by impulse/mass") {
    Particle p(2.0f, 0.1f, glm::vec3(0.0f), glm::vec3(0.0f));

    glm::vec3 impulse(0.02f, 0.04f, 0.06f);
    p.applyImpulse(impulse);

    // Δv = J / m = (0.02,0.04,0.06)/2 = (0.01,0.02,0.03)
    requireVecApprox(p.velocity, glm::vec3(0.01f, 0.02f, 0.03f), 1e-7f);
}

TEST_CASE("applyImpulse ignores zero impulse") {
    Particle p(1.0f, 0.1f, glm::vec3(0.0f), glm::vec3(1.0f, 2.0f, 3.0f));
    p.applyImpulse(glm::vec3(0.0f));
    requireVecApprox(p.velocity, glm::vec3(1.0f, 2.0f, 3.0f));
}

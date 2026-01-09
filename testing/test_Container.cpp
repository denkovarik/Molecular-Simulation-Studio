// testing/test_Container.cpp

/* 
Usage:

g++ -std=c++14 -I/usr/local/include -o test_Container.exe \
    testing/test_Container.cpp src/classes/Particle.cpp src/classes/Container.cpp

./test_Container.exe

*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "../src/classes/Container.hpp"
#include <iostream>

TEST_CASE("Container constructor") {
    Container c1 = Container();
}

TEST_CASE("Container with 1 Particle no Wall Collisions") {
    Container c1 = Container();
    c1.particles.push_back(Particle(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.02f, 0.0f, 0.0f)));
    
    c1.checkWallCollisions();
    
    REQUIRE(c1.particles[0].velocity.x == 0.02f);
    REQUIRE(c1.particles[0].velocity.y == 0.0f);
    REQUIRE(c1.particles[0].velocity.z == 0.0f);

}

TEST_CASE("Container with Particle with Wall Collisions") {
    Container c1 = Container();
    
    // Right Wall Collision
    c1.particles.push_back(Particle(1.0f, 0.1f, glm::vec3(c1.rightWallX, 0.0f, 0.0f), glm::vec3(0.02f, 0.0f, 0.0f)));
    // Left Wall Collision
    c1.particles.push_back(Particle(1.0f, 0.1f, glm::vec3(c1.leftWallX, 0.0f, 0.0f), glm::vec3(-0.02f, 0.0f, 0.0f)));
    // Ceiling Collision
    c1.particles.push_back(Particle(1.0f, 0.1f, glm::vec3(0.0f, c1.ceilingY, 0.0f), glm::vec3(0.0f, 0.2f, 0.0f)));
    // Floor Collision
    c1.particles.push_back(Particle(1.0f, 0.1f, glm::vec3(0.0f, c1.floorY, 0.0f), glm::vec3(0.0f, -0.2f, 0.0f)));
    // Front Collision
    c1.particles.push_back(Particle(1.0f, 0.1f, glm::vec3(0.0f, 0.0f, c1.frontWallZ), glm::vec3(0.0f, 0.0f, 0.01f)));
    // Back Collision
    c1.particles.push_back(Particle(1.0f, 0.1f, glm::vec3(0.0f, 0.0f, c1.backWallZ), glm::vec3(0.0f, 0.0f, -0.01f)));

    c1.checkWallCollisions();

    // Right Wall Collision
    int i = 0;
    REQUIRE(c1.particles[i].position == glm::vec3(1.9f, 0.0f, 0.0f));
    REQUIRE(c1.particles[i].velocity == glm::vec3(-0.02f, 0.0f, 0.0f));
    
    // Left Wall Collision
    i = 1;
    REQUIRE(c1.particles[i].position == glm::vec3(-1.9f, 0.0f, 0.0f));
    REQUIRE(c1.particles[i].velocity == glm::vec3(0.02f, 0.0f, 0.0f));
    
    // Ceiling Collision
    i = 2;
    REQUIRE(c1.particles[i].position == glm::vec3(0.0f, 1.9f, 0.0f));
    REQUIRE(c1.particles[i].velocity == glm::vec3(0.0f, -0.2f, 0.0f));
    
    // Floor Collision
    i = 3;
    REQUIRE(c1.particles[i].position == glm::vec3(0.0f, -1.9f, 0.0f));
    REQUIRE(c1.particles[i].velocity == glm::vec3(0.0f, 0.2f, 0.0f));
    
    // Front Collision
    i = 4;
    REQUIRE(c1.particles[i].position == glm::vec3(0.0f, 0.0f, 1.9f));
    REQUIRE(c1.particles[i].velocity == glm::vec3(0.0f, 0.0f, -0.01f));
    
    // Back Collision
    i = 5;
    REQUIRE(c1.particles[i].position == glm::vec3(0.0f, 0.0f, -1.9f));
    REQUIRE(c1.particles[i].velocity == glm::vec3(0.0f, 0.0f, 0.01f));
}

TEST_CASE("Container: resolveParticleCollision should separate two equal-mass particles in head-on collision") {
    Container c;

    // Arrange: two equal-mass particles overlapping and moving toward each other along x
    Particle a(1.0f, 0.5f, glm::vec3(-0.4f, 0.0f, 0.0f), glm::vec3(+1.0f, 0.0f, 0.0f));
    Particle b(1.0f, 0.5f, glm::vec3(+0.4f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f));

    // Sanity: they should be overlapping so collision resolution triggers
    REQUIRE(c.particlesCollide(a, b));

    // Total momentum before (should be 0)
    glm::vec3 p_before = a.mass * a.velocity + b.mass * b.velocity;
    REQUIRE(p_before.x == Approx(0.0f).margin(1e-6f));

    // Act
    c.resolveParticleCollision(a, b);

    // Assert: after an elastic head-on collision of equal masses,
    // they should separate: a should move left, b should move right.
    REQUIRE(a.velocity.x < 0.0f);
    REQUIRE(b.velocity.x > 0.0f);

    // Momentum should still be conserved
    glm::vec3 p_after = a.mass * a.velocity + b.mass * b.velocity;
    REQUIRE(p_after.x == Approx(p_before.x).margin(1e-6f));
    REQUIRE(p_after.y == Approx(p_before.y).margin(1e-6f));
    REQUIRE(p_after.z == Approx(p_before.z).margin(1e-6f));

    // For equal masses and perfectly elastic, speeds should match original magnitude
    REQUIRE(std::abs(a.velocity.x) == Approx(1.0f).margin(1e-6f));
    REQUIRE(std::abs(b.velocity.x) == Approx(1.0f).margin(1e-6f));
}

TEST_CASE("Container grid cells have increasing min/max bounds (xMin < xMax, etc)") {
    Container c; // default container constructs grid

    // Basic sanity: grid should exist
    REQUIRE(c.theGrid.theMatrix.size() > 0);
    REQUIRE(c.theGrid.theMatrix[0].size() > 0);
    REQUIRE(c.theGrid.theMatrix[0][0].size() > 0);

    const Cell& cell = c.theGrid.theMatrix[0][0][0];

    // These MUST be true for a well-formed axis-aligned cell.
    REQUIRE(cell.xMin < cell.xMax);
    REQUIRE(cell.yMin < cell.yMax);
    REQUIRE(cell.zMin < cell.zMax);
}



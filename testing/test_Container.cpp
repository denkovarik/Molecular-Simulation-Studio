// g++ -std=c++14 -I/usr/local/include/catch2 -L/usr/local/lib -o test_Container.exe testing/test_Container.cpp src/classes/Particle.cpp src/classes/Container.cpp -lCatch2Main -lCatch2
// ./test_Container.exe

#define CATCH_CONFIG_MAIN  // This tells Catch2 to provide a main function
#include "catch2/catch_all.hpp"
#include "catch2/catch_approx.hpp"
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
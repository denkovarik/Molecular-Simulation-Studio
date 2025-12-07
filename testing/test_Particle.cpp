// g++ -std=c++14 -I/usr/local/include/catch2 -L/usr/local/lib -o test_Particle.exe testing/test_Particle.cpp src/classes/Particle.cpp -lCatch2Main -lCatch2
// ./test_Particle.exe

#define CATCH_CONFIG_MAIN  // This tells Catch2 to provide a main function
#include "catch2/catch_all.hpp"
#include "catch2/catch_approx.hpp"
#include "../src/classes/Particle.hpp"
#include <iostream>

TEST_CASE("Particle constructor") {
    Particle p1 = Particle(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));   
    
    REQUIRE(p1.position.x == 0.0);
    REQUIRE(p1.position.y == 0.0);
    REQUIRE(p1.position.z == 0.0);
    REQUIRE(p1.velocity.x == 0.0);
    REQUIRE(p1.velocity.y == 0.0);
    REQUIRE(p1.velocity.z == 0.0);
    
    Particle p2 = Particle(2, 0.5f, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)); 
    
    REQUIRE(p2.mass == 2);
    REQUIRE(p2.radius == 0.5);
}

TEST_CASE("Update Particle Velocity Given Applied Force") {
    Particle p1 = Particle(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));  
    
    glm::vec3 force = glm::vec3(0.01f, 0.02f, 0.03f); // force    
    
    p1.applyForce(force);
    
    REQUIRE(p1.velocity.x == 0.01f);
    REQUIRE(p1.velocity.y == 0.02f);
    REQUIRE(p1.velocity.z == 0.03f);
}

TEST_CASE("Update Particle Position") {
    Particle p1 = Particle(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));  
    
    glm::vec3 force = glm::vec3(0.01f, 0.02f, 0.03f); // force    
    
    p1.applyForce(force);
    p1.updatePosition();
    
    REQUIRE(p1.position.x == 0.01f);
    REQUIRE(p1.position.y == 0.02f);
    REQUIRE(p1.position.z == 0.03f);
}
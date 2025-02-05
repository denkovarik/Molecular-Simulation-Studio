// g++ -std=c++14 -I/usr/local/include/catch2 -L/usr/local/lib -o test_Simulation.exe testing/test_Simulation.cpp src/classes/SubAtomicParticle.cpp src/classes/Simulation.cpp -lCatch2Main -lCatch2

#include "../src/classes/Simulation.hpp"
#include "catch2/catch_all.hpp"
#include "catch2/catch_approx.hpp"
#include "../src/classes/SubAtomicParticle.hpp"
#include <iostream>


TEST_CASE("Simulation: Add particle") {
    Simulation simulation = Simulation();
    
    // Create an instance of SubAtomicParticle
    std::vector<double> position = {0.0, 0.0, 0.0};
    std::vector<double> velocity = {1.0, 2.0, 3.0};
    double mass = 9.109e-31; // Mass of electron in kg
    double charge = -1.602e-19; // Charge of an electron in Coulombs
    int spin = 1 / 2; // Spin of an electron
    SubAtomicParticle particle(mass, position, velocity, charge, spin);    
    
    simulation.addParticle(particle);

    // Check that the particle was added successfully
    std::vector<SubAtomicParticle> particles = simulation.getParticles();
    REQUIRE(particles.size() == 1);
}

TEST_CASE("Simulation: Update particles with no forces applied to them") {
    Simulation simulation;
    
    // Create an instance of SubAtomicParticle
    std::vector<double> position = {0.0, 0.0, 0.0};
    std::vector<double> velocity = {1.0, 2.0, 3.0};
    double mass = 1.6726e-27; // Mass of proton in kg
    double charge = 1.602e-19; // Charge of an proton in Coulombs
    int spin = 1 / 2; // Spin of an electron
    SubAtomicParticle proton = SubAtomicParticle(mass, position, velocity, charge, spin);
    std::vector<double> initialPosition = proton.getPosition();
    simulation.addParticle(proton);

    // Update the simulation
    double deltaTime = 1.0;
    simulation.update(deltaTime);

    double expectedPositionX = 1;
    double expectedPositionY = 2;
    double expectedPositionZ = 3;
    
    std::vector<SubAtomicParticle> particlesUpdated = simulation.getParticles();
    REQUIRE(particlesUpdated.size() == 1);
    proton = particlesUpdated[0];
    std::vector<double> expectedPosition = {1,2,3};    
    std::vector<double> newPosition = proton.getPosition();

    REQUIRE(newPosition == expectedPosition);
}
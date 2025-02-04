#define CATCH_CONFIG_MAIN  // This tells Catch2 to provide a main function
#include "catch2/catch_all.hpp"
#include "catch2/catch_approx.hpp"
#include "../src/classes/SubAtomicParticle.hpp"

TEST_CASE("SubAtomicParticle constructor") {
    // Create an instance of SubAtomicParticle
    std::vector<double> position = {0.0, 0.0, 0.0};
    std::vector<double> velocity = {1.0, 2.0, 3.0};
    double mass = 9.109e-31; // Mass of electron in kg
    double charge = -1.602e-19; // Charge of an electron in Coulombs
    int spin = 1; // Spin of an electron, corrected to avoid integer division

    SubAtomicParticle particle(mass, position, velocity, charge, spin);
    
    // Use Approx for floating point comparison
    REQUIRE(particle.getCharge() == -1.602e-19);
}
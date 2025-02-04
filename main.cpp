#include "src/classes/SubAtomicParticle.hpp"
#include <iostream>
#include <vector>

int main() {
    // Create an instance of SubAtomicParticle
    std::vector<double> position = {0.0, 0.0, 0.0};
    std::vector<double> velocity = {1.0, 2.0, 3.0};
    double mass = 9.109e-31; // Mass of electron in kg
    double charge = -1.602e-19; // Charge of an electron in Coulombs
    int spin = 1 / 2; // Spin of an electron

    SubAtomicParticle particle(mass, position, velocity, charge, spin);

    // Set the force acting on the particle (for example, a constant electric field)
    std::vector<double> force = {0.5e-9, 0.0, 0.0}; // N
    particle.setForces(force);

    std::cout << "Mass: " << particle.getMass() << " kg" << std::endl;
    std::cout << "Charge: " << particle.getCharge() << " C" << std::endl;
    std::cout << "Spin: " << particle.getSpin() << std::endl;

    // Print the position, velocity, force
    std::vector<double> pos = particle.getPosition();
    std::vector<double> vel = particle.getVelocity();

    std::cout << "Position (m): (" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")" << std::endl;
    std::cout << "Velocity (m/s): (" << vel[0] << ", " << vel[1] << ", " << vel[2] << ")" << std::endl;
    std::cout << "Force (N): (" << force[0] << ", " << force[1] << ", " << force[2] << ")" << std::endl;

    return 0;
}
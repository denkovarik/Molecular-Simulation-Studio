#include "SubAtomicParticle.hpp"
#include <cmath>
#include <iostream>
#include <vector>

SubAtomicParticle::SubAtomicParticle(double mass, std::vector<double> position, std::vector<double>& velocity, double charge, int spin) 
    : mass_(mass), charge_(charge), spin_(spin)
{
    position_ = position;
    velocity_ = velocity;
    
    // Initialize force vector with zeros (no net force initially)
    std::vector<double> zeroForce(3, 0.0);
    forces_ = zeroForce;
}

double SubAtomicParticle::getMass() const { return mass_; }
double SubAtomicParticle::getCharge() const { return charge_; }
int SubAtomicParticle::getSpin() const { return spin_; }
std::vector<double> SubAtomicParticle::getPosition() const { return position_; }
std::vector<double> SubAtomicParticle::getVelocity() const { return velocity_; }
std::vector<double> SubAtomicParticle::getForces() const { return forces_; }

void SubAtomicParticle::setForces(std::vector<double>& forces) {
    forces_ = forces;
}
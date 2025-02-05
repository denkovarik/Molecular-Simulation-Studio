#include "Simulation.hpp"
#include <iostream>

Simulation::Simulation() {}

void Simulation::addParticle(SubAtomicParticle particle)
{
    particles_.push_back(particle);
}

void Simulation::update(double deltaTime)
{
    for(int i = 0; i < particles_.size(); i++)
    {
        // Update 
        particles_[i].update(deltaTime);
    }
}

std::vector<SubAtomicParticle> Simulation::getParticles()
{
    return particles_;
}
// src/classes/Simulation.hpp
#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include "config.hpp"
#include "Container.hpp"
#include "Particle.hpp"
#include <vector>
#include "config.hpp"

class Simulation {
public:
    explicit Simulation(const Config& cfg);

    void update(float dt);

    const std::vector<Particle>& getParticles() const { return container.particles; }
    
    Container container;
    Config config;

private:
    void computeChemicalForces(float dt);

};

#endif // SIMULATION_HPP

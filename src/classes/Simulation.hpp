// src/classes/Simulation.hpp
#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include "../config.hpp"
#include "Container.hpp"
#include "Particle.hpp"
#include "Atom.hpp"
#include <vector>
#include <map>
#include <string>

struct ElementData { 
    int Z;
    float mass;
    float charge;
    float vdw_r;
    float harmonic_k;
    float harmonic_eq;
    float barrier;
};

class Simulation {
public:
    explicit Simulation(const Config& cfg);
    void computeCoulombForces(float dt);
    void update(float dt);
    const std::vector<Particle>& getParticles() const { return container.particles; }
   
    Container container;
    Config config;

    std::vector<Atom> atoms;
    void checkReactions();

    std::map<std::string, ElementData> elementData;  
    void loadElementsFromJSON(const std::string& filename);  

private:
    void computeChemicalForces(float dt);
};

#endif // SIMULATION_HPP

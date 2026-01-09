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

struct Bond {
    int i = -1;
    int j = -1;
    float r0 = 0.74f;
    float De = 10.0f;
    float a  = 8.0f;
    float strength = 1.0f;  
};

class Simulation {
public:
    explicit Simulation(const Config& cfg);
    void computeCoulombForces(float dt);
    void update(float dt);
    const std::vector<Particle>& getParticles() const { return container.particles; }
    std::vector<Bond> bonds;
   
    Container container;
    Config config;

    std::vector<Atom> atoms;
    void checkReactions();

    std::map<std::string, ElementData> elementData;  
    void loadElementsFromJSON(const std::string& filename);  

private:
    void computeChemicalForces(float dt);
    void tryFormBonds();

};

#endif // SIMULATION_HPP

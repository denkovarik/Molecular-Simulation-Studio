#ifndef ATOM_HPP
#define ATOM_HPP

#include "Particle.hpp"
#include "SubAtomicParticle.hpp"
#include "../config.hpp"
#include <vector>
#include <string>
#include <glm/glm.hpp>

class Atom 
{
public:
    Particle nucleus;
    std::vector<SubAtomicParticle> electrons;
    std::string elementSymbol;
    int atomicNumber;
    float vanDerWaalsRadius;
    float harmonic_k;
    float harmonic_eq;
    float morse_De;
    float morse_a;
    std::vector<int> bonded_to; // Indices of bonded atoms
    float bond_strength = 0.0f; // Bond energy (eV)
         
    Atom(const std::string& symbol, int Z, float mass, float charge, float vdw_r, float h_k, float h_eq,
         float morse_De, float morse_a, const glm::vec3& pos, const glm::vec3& vel);

    void update(float dt);
    void applyForces(std::vector<Atom>& others, float dt, const Config& cfg);  // Changed to non-const
};

#endif

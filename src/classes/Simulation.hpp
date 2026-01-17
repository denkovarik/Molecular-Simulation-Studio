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
#include "third_party/nlohmann/json.hpp"

class QmSurface1D;

struct Reaction {
    std::string name;
    std::vector<std::string> reactants;
    std::string product;
    float min_dist;
    float min_ke;
    float bond_energy;
};

struct ElementData 
{ 
    int Z;
    float mass;
    float charge;
    float vdw_r;
    float harmonic_k;
    float harmonic_eq;
    float barrier;
};

struct Bond 
{
    int i = -1;
    int j = -1;
    float r0 = 0.74f;
    float De = 10.0f;
    float a  = 8.0f;
    float strength = 1.0f;  
};

class Simulation 
{
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
    std::vector<Reaction> reactions;
    void loadReactionsFromJSON(const std::string& filename);
    void setQmSurfaceH2(const QmSurface1D* surf);    
    void computeChemicalForces(float dt);
    void enforceValencyAndBreakBonds();

private:
    void tryFormBonds();
    const QmSurface1D* qm_surface_h2_ = nullptr;
    double compute_qm_energy(double r_bohr) const;  // Wrapper for compute_energy
    glm::vec3 compute_qm_force(const Atom& a, const Atom& b) const;
    void applyQmSurfaceH2Forces(float dt);
    void applyQmForces(float dt);
};

#endif // SIMULATION_HPP

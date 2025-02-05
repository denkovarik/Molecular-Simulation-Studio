#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include "SubAtomicParticle.hpp"
#include <vector>

class Simulation {
    public:
        Simulation();

        void addParticle(SubAtomicParticle particle);  

        void update(double deltaTime);

        std::vector<SubAtomicParticle> getParticles();
    
    private:
        std::vector<SubAtomicParticle> particles_;  // Vector of particles in the simulation
};

#endif  // SIMULATION_HPP
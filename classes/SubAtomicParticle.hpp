#ifndef SUBATOMICPARTICLE_HPP
#define SUBATOMICPARTICLE_HPP

#include <vector>

class SubAtomicParticle 
{
    public:
        // Constructor
        SubAtomicParticle(double mass, std::vector<double> position, std::vector<double>& velocity, double charge, int spin);

        // Getters and setters for properties
        double getMass() const;
        double getCharge() const; 
        int getSpin() const;
        std::vector<double> getPosition() const;
        std::vector<double> getVelocity() const;
        std::vector<double> getForces() const;
        void setForces(std::vector<double>& force);

    private:
        double mass_;
        double charge_;
        int spin_;
        std::vector<double> position_;
        std::vector<double> velocity_;
        std::vector<double> forces_;
};

#endif  // SUBATOMICPARTICLE_HPP
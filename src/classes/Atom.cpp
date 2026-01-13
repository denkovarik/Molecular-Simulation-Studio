#include "Atom.hpp"
#include "../physics/Coulomb.hpp"
#include <glm/gtx/norm.hpp>

Atom::Atom(const std::string& symbol, int Z, float mass, float charge, float vdw_r,
           float h_k, float h_eq, const glm::vec3& pos, const glm::vec3& vel)
    : nucleus(mass, vdw_r / 2.0f, charge, pos, vel),
      electrons(),
      elementSymbol(symbol),
      atomicNumber(Z),
      vanDerWaalsRadius(vdw_r),
      harmonic_k(h_k),
      harmonic_eq(h_eq)
{}

void Atom::update(float dt) 
{
    nucleus.updatePosition(dt);
    for (auto& e : electrons) 
    {
        e.update(dt);
    }
}

void Atom::applyForces(std::vector<Atom>& others, float dt, const Config& cfg) 
{
    // Binding forces
    for (auto& e : electrons) {
        glm::vec3 r = glm::vec3(
                        e.getPosition()[0], e.getPosition()[1], e.getPosition()[2]
                      ) - nucleus.position;
        float dist = glm::length(r);
        if (dist > 1e-6f) 
        {
            float disp = dist - harmonic_eq;
            glm::vec3 F = -harmonic_k * disp * (r / dist);
            std::vector<double> vel_e = e.getVelocity();
            glm::vec3 dv_e = F * (dt / cfg.electron_mass);
            vel_e[0] += dv_e.x;
            vel_e[1] += dv_e.y;
            vel_e[2] += dv_e.z;
            e.setVelocity(vel_e);
            nucleus.velocity -= dv_e * (cfg.electron_mass / nucleus.mass);
        }
    }
   
    // Electron-other nucleus
    for (auto& other : others) 
    {
        if (&other == this) continue;
        for (auto& e : electrons) 
        {
            glm::vec3 r = other.nucleus.position 
                        - glm::vec3(e.getPosition()[0], e.getPosition()[1], e.getPosition()[2]);
            glm::vec3 F_on_e = coulombForce(r, e.getCharge(), other.nucleus.charge, 
                                            cfg.coulomb_k, cfg.coulomb_softening);
            std::vector<double> vel_e = e.getVelocity();
            glm::vec3 dv_e = F_on_e * (dt / cfg.electron_mass);
            vel_e[0] += dv_e.x; vel_e[1] += dv_e.y; vel_e[2] += dv_e.z;
            e.setVelocity(vel_e);
            // Equal-and-opposite on other nucleus
            other.nucleus.velocity -= F_on_e * dt / other.nucleus.mass;
        }
    }
   
    // Coulomb with other nuclei
    for (auto& other : others) 
    {
        if (&other == this) continue;
        glm::vec3 r = other.nucleus.position - nucleus.position;
        glm::vec3 F = coulombForce(r, nucleus.charge, other.nucleus.charge, 
                                   cfg.coulomb_k, cfg.coulomb_softening);
        nucleus.velocity += F * (dt / nucleus.mass);
    }
   
    // Damping
    nucleus.velocity *= 0.99f;
    for (auto& e : electrons) 
    {
        std::vector<double> vel_e = e.getVelocity();
        vel_e[0] *= 0.99f;
        vel_e[1] *= 0.99f;
        vel_e[2] *= 0.99f;
        e.setVelocity(vel_e);
    }
}

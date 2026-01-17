// src/classes/Simulation.cpp

#include <iostream>
#include "Simulation.hpp"
#include <random>
#include <cmath>
#include <stdexcept>
#include <string>
#include <algorithm> 
#include <glm/glm.hpp>
#include "../physics/Coulomb.hpp"
#include "third_party/nlohmann/json.hpp"
#include <fstream>  
#include "../physics/QmSurface1D.hpp"

using json = nlohmann::json;  // Alias

// Constructor: Set up container and generate particles
Simulation::Simulation(const Config& cfg)
    : container(cfg.containerMaxX, cfg.containerMinX,
                cfg.containerMaxY, cfg.containerMinY,
                cfg.containerMaxZ, cfg.containerMinZ),
      config(cfg)
{
    if (config.enable_subatomic) 
    {
        // Subatomic mode: Spawn Atoms instead
        atoms.reserve(config.numParticles);
        // For test/demo: caller will populate, or add random spawn logic here
        return;
    }

    // Legacy mode: Particles
    if (!cfg.spawnRandomParticles || cfg.numParticles <= 0) {
        return; // caller will populate container.particles manually
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> x_dist(cfg.containerMinX + cfg.particleRadius * 2,
                                                 cfg.containerMaxX - cfg.particleRadius * 2);
    std::uniform_real_distribution<float> y_dist(cfg.containerMinY + cfg.particleRadius * 2,
                                                 cfg.containerMaxY - cfg.particleRadius * 2);
    std::uniform_real_distribution<float> z_dist(cfg.containerMinZ + cfg.particleRadius * 2,
                                                 cfg.containerMaxZ - cfg.particleRadius * 2);
    std::uniform_real_distribution<float> vel_dist(-cfg.velocityRange, cfg.velocityRange);
    for (int n = 0; n < cfg.numParticles; ++n) 
    {
        bool placed = false;
        for (int attempt = 0; attempt < cfg.maxPlacementAttemptsPerParticle; ++attempt) 
        {
            glm::vec3 pos(x_dist(gen), y_dist(gen), z_dist(gen));
            glm::vec3 vel(vel_dist(gen), vel_dist(gen), vel_dist(gen));
            Particle newParticle(cfg.particleMass, cfg.particleRadius, pos, vel);
            bool collides = false;
            for (const auto& p : container.particles) 
            {
                if (container.particlesCollide(newParticle, p)) 
                {
                    collides = true;
                    break;
                }
            }
            if (!collides) 
            {
                container.particles.push_back(newParticle);
                placed = true;
                break;
            }
        }
        if (!placed) 
        {
            throw std::runtime_error(
                "Simulation init: failed to place particle " + std::to_string(n) +
                " without overlap. Reduce numParticles, reduce radius, or enlarge container."
            );
        }
    }
}

void Simulation::loadElementsFromJSON(const std::string& filename) 
{
    std::ifstream file(filename);
    if (!file.is_open()) 
    {
        throw std::runtime_error("Failed to open JSON file: " + filename);
    }
    json j;
    file >> j;
    elementData.clear();
    for (auto& [key, value] : j.items()) 
    {
        ElementData data;
        data.Z = value["Z"];
        data.mass = value["mass"];
        data.charge = value["charge"];
        data.vdw_r = value["vdw_r"];
        data.harmonic_k = value["harmonic_k"];
        data.harmonic_eq = value["harmonic_eq"];
        data.barrier = value["barrier"];
        elementData[key] = data;
    }
}

static bool alreadyBonded(const std::vector<Bond>& bonds, int i, int j) 
{
    if (i > j) std::swap(i, j);
    for (const auto& b : bonds) 
    {
        int a = b.i, c = b.j;
        if (a > c) std::swap(a, c);
        if (a == i && c == j) return true;
    }
    return false;
}

void Simulation::computeChemicalForces(float dt)
{
    auto& ps = container.particles;
    if (dt <= 0.0f) return;
    for (size_t i = 0; i < ps.size(); ++i)
    {
        for (size_t j = i + 1; j < ps.size(); ++j)
        {
            Particle& a = ps[i];
            Particle& b = ps[j];
            glm::vec3 r = b.position - a.position;
            float r2 = glm::dot(r, r);
            if (r2 < 1e-12f) continue;
            float dist = std::sqrt(r2);
            glm::vec3 u = r / dist; // unit vector
            glm::vec3 F(0.0f);
            if (alreadyBonded(bonds, i, j)) {
                // Morse potential force (on a)
                float exp_term = std::exp(-config.bond_a * (dist - config.bond_r0));
                float f_mag = 2.0f * config.bond_De * config.bond_a * exp_term * (1.0f - exp_term);
                F = f_mag * u; // attractive/repulsive to hold at r0
            } else {
                // Lennard-Jones for non-bonded
                float sr = config.lj_sigma / dist;
                float sr6 = std::pow(sr, 6.0f);
                float sr12 = std::pow(sr, 12.0f);
                float f_over_r = 24.0f * config.lj_epsilon * (sr6 - 2.0f * sr12) / (dist * dist);
                F = f_over_r * r;
            }
            a.velocity += (F / a.mass) * dt;
            b.velocity -= (F / b.mass) * dt;
        }
    }
}

void Simulation::computeCoulombForces(float dt) 
{
    if (!config.enable_coulomb) return;
    if (dt <= 0.0f) return;
    auto& ps = container.particles;
    // Softening must be > 0 to avoid singularities if someone sets it to 0
    const float soft = std::max(config.coulomb_softening, 1e-6f);
    for (size_t i = 0; i < ps.size(); ++i) 
    {
        for (size_t j = i + 1; j < ps.size(); ++j) 
        {
            Particle& a = ps[i];
            Particle& b = ps[j];
            glm::vec3 r_ab = b.position - a.position;
            // Force ON a due to b
            glm::vec3 F_on_a = coulombForce(
                r_ab,
                a.charge, b.charge,
                config.coulomb_k,
                soft
            );
            // Semi-implicit Euler: update v from forces
            a.velocity += (F_on_a / a.mass) * dt;
            b.velocity -= (F_on_a / b.mass) * dt; // equal-and-opposite
        }
    }
}

void Simulation::checkReactions() {
    for (size_t i = 0; i < atoms.size(); ++i) {
        for (size_t j = i + 1; j < atoms.size(); ++j) {
            glm::vec3 r = atoms[j].nucleus.position - atoms[i].nucleus.position;
            float dist = glm::length(r);
            glm::vec3 rel_vel = atoms[j].nucleus.velocity - atoms[i].nucleus.velocity;
            float reduced_mass = (atoms[i].nucleus.mass * atoms[j].nucleus.mass) / (atoms[i].nucleus.mass + atoms[j].nucleus.mass);
            float rel_ke = 0.5f * reduced_mass * glm::dot(rel_vel, rel_vel);
            bool rule_matched = false;
            for (const auto& rule : reactions) {
                if (rule.reactants.size() == 2 &&
                    ((atoms[i].elementSymbol == rule.reactants[0] && atoms[j].elementSymbol == rule.reactants[1]) ||
                     (atoms[i].elementSymbol == rule.reactants[1] && atoms[j].elementSymbol == rule.reactants[0])) &&
                    dist < rule.min_dist && rel_ke > rule.min_ke) {
                    rule_matched = true;
                    atoms[i].bonded_to.push_back(j);
                    atoms[j].bonded_to.push_back(i);
                    atoms[i].bond_strength = rule.bond_energy;
                    atoms[j].bond_strength = rule.bond_energy;
                    glm::vec3 mid = (atoms[i].nucleus.position + atoms[j].nucleus.position) / 2.0f;
                    glm::vec3 dir = glm::normalize(r);
                    atoms[i].nucleus.position = mid - dir * 0.37f;
                    atoms[j].nucleus.position = mid + dir * 0.37f;
                    atoms[i].nucleus.velocity *= 0.95f;
                    atoms[j].nucleus.velocity *= 0.95f;
                    break;
                }
            }
            if (!rule_matched && atoms[i].elementSymbol == "H" && atoms[j].elementSymbol == "H" &&
                dist < 1.5f && rel_ke > config.activation_barrier) {
                atoms[i].bonded_to.push_back(j);
                atoms[j].bonded_to.push_back(i);
                atoms[i].bond_strength = config.bond_energy;  
                atoms[j].bond_strength = config.bond_energy;
                glm::vec3 mid = (atoms[i].nucleus.position + atoms[j].nucleus.position) / 2.0f;
                glm::vec3 dir = glm::normalize(r);
                atoms[i].nucleus.position = mid - dir * 0.37f;
                atoms[j].nucleus.position = mid + dir * 0.37f;
                atoms[i].nucleus.velocity *= 0.5f;
                atoms[j].nucleus.velocity *= 0.5f;
            }
        }
    }
}

void Simulation::update(float dt) 
{
    if (config.enable_subatomic) 
    {
        if (config.enable_subatomic) {
            if (config.enable_qm_surface_h2 || config.enable_dynamic_qm) {
                applyQmForces(dt);
            }
            if (config.enable_atom_forces) {
                for (auto& atom : atoms) {
                    atom.applyForces(atoms, dt, config);
                }
            }
            checkReactions();
            for (auto& atom : atoms) {
                atom.update(dt);
            }
            return;
        }
        // Otherwise, use existing classical-ish atom forces (optional)
        if (config.enable_atom_forces) {
            for (auto& atom : atoms) {
                atom.applyForces(atoms, dt, config);
            }
        }

        checkReactions();

        for (auto& atom : atoms) {
            atom.update(dt);
        }
        return;
    }

    // Legacy mode: Particles
    // Forces (can coexist)
    if (config.enable_chemistry) computeChemicalForces(dt);
    computeCoulombForces(dt);

    tryFormBonds();

    // Spatial / collisions
    container.assignParticles2Grid();
    if (!config.enable_chemistry) 
    {
        // Keep old "bouncing balls" behavior when chemistry is off
        container.resolveParticleCollisions();
    }
    if (config.enable_damping) 
    {
        container.checkWallCollisions(config.wall_collision_dampening);
    }
    else 
    {
        container.checkWallCollisions();
    }
    container.checkWallCollisions();
    // Integrate positions
    for (auto& p : container.particles) 
    {
        p.position += p.velocity * dt;
    }
}

void Simulation::tryFormBonds()
{
    if (!config.enable_bonds) return;
    auto& ps = container.particles;
    const float r_form = config.bond_form_dist;
    
    enforceValencyAndBreakBonds();  // Clean first

    for (int i = 0; i < (int)ps.size(); ++i)
    {
        int bondCountI = 0;
        for (const auto& b : bonds) if (b.i == i || b.j == i) bondCountI++;
        if (bondCountI > 0) continue;  // Strictly free only (no bonds at all)

        for (int j = i + 1; j < (int)ps.size(); ++j)
        {
            int bondCountJ = 0;
            for (const auto& b : bonds) if (b.i == j || b.j == j) bondCountJ++;
            if (bondCountJ > 0) continue;  // Strictly free only

            if (ps[i].elementSymbol != "H" || ps[j].elementSymbol != "H") continue;

            glm::vec3 r = ps[j].position - ps[i].position;
            float dist = glm::length(r);
            if (dist > r_form) continue;
            glm::vec3 vrel = ps[j].velocity - ps[i].velocity;
            if (glm::dot(vrel, r) >= 0.0f) continue;
            if (alreadyBonded(bonds, i, j)) continue;

            Bond b;
            b.i = i;
            b.j = j;
            b.r0 = config.bond_r0;
            b.De = config.bond_De;
            b.a = config.bond_a;
            b.strength = 1.0f;
            bonds.push_back(b);

            // Enforce immediately after add
            enforceValencyAndBreakBonds();

            //return;  // One per update
        }
    }
}

void Simulation::enforceValencyAndBreakBonds() {
    bool changed = true;
    while (changed) {
        changed = false;
        // Break stretched
        for (auto it = bonds.begin(); it != bonds.end(); ) {
            int i = it->i, j = it->j;
            float dist = glm::length(container.particles[j].position 
                       - container.particles[i].position);
            if (dist > config.bond_break_dist) {
                it = bonds.erase(it);
                changed = true;
            } else {
                ++it;
            }
        }
        // Break excess valency (keep shortest bonds)
        for (int k = 0; k < (int)container.particles.size(); ++k) {
            std::vector<std::pair<float, size_t>> particleBonds;  // <dist, bond_index>
            for (int idx = 0; idx < (int)bonds.size(); ++idx) {
                const auto& b = bonds[idx];
                if (b.i == k || b.j == k) {
                    int other = (b.i == k) ? b.j : b.i;
                    float dist = glm::length(container.particles[k].position 
                               - container.particles[other].position);
                    particleBonds.emplace_back(dist, idx);
                }
            }
            if (particleBonds.size() <= static_cast<size_t>(config.maxValence)) continue;

            // Sort ascending by dist (shortest first)
            std::sort(particleBonds.begin(), particleBonds.end());
            // Break longest (from end)
            while (particleBonds.size() > static_cast<size_t>(config.maxValence)) {
                size_t eraseIdx = particleBonds.back().second;
                bonds.erase(bonds.begin() + eraseIdx);
                particleBonds.pop_back();
                changed = true;
            }
        }
    }
}

void Simulation::loadReactionsFromJSON(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Failed to open " + filename);
    json j;
    file >> j;
    reactions.clear();
    for (auto& r : j["reactions"]) {
        Reaction rule;
        rule.name = r["name"];
        rule.reactants = r["reactants"].get<std::vector<std::string>>();
        rule.product = r["product"];
        rule.min_dist = r["min_dist"];
        rule.min_ke = r["min_ke"];
        rule.bond_energy = r["bond_energy"];
        reactions.push_back(rule);
    }
}

void Simulation::setQmSurfaceH2(const QmSurface1D* surf) {
    qm_surface_h2_ = surf;
}

// Include <libint2/initialize.h> etc. or forward to h2_qm_sim
double Simulation::compute_qm_energy(double r_bohr) const {
    // Wrapper for compute_energy(r_bohr, config.use_dft) from h2_qm_sim.cpp
    // For test: use toy analytic (Morse-like) to pass without full QM deps
    // Real: call actual QM
    double De = 4.5;  // Bond energy (eV-ish)
    double a = 1.0;
    double re = 0.74 / 0.529;  // bohr
    return De * (1.0 - std::exp(-a * (r_bohr - re))) * (1.0 - std::exp(-a * (r_bohr - re))); 
}

glm::vec3 Simulation::compute_qm_force(const Atom& a, const Atom& b) const {
    glm::vec3 r = b.nucleus.position - a.nucleus.position;
    float R_ang = glm::length(r);  // Å
    if (R_ang < 1e-8f) return glm::vec3(0.0f);
    glm::vec3 u = r / R_ang;
    double r_bohr = R_ang / 0.529;  // To bohr
    // Finite diff gradient (dE/dr)
    const double eps = 1e-5;
    double E0 = compute_qm_energy(r_bohr);
    double E_plus = compute_qm_energy(r_bohr + eps);
    double dEdr = (E_plus - E0) / eps;  // Hartree/bohr
    // Convert to force units (adjust scale for your sim; e.g., *27.211 for eV)
    float force_mag = static_cast<float>(dEdr * 27.211 / 0.529);  // Rough eV/Å
    return force_mag * u;  // F_on_a
}

void Simulation::applyQmSurfaceH2Forces(float dt) {
    std::cout << "In applyQmSurfaceH2Forces" << std::endl;
    
    if (atoms.size() != 2) return;
    if (dt <= 0.0f) return;
    auto& a = atoms[0].nucleus;
    auto& b = atoms[1].nucleus;
    glm::vec3 r = b.position - a.position;
    float R = glm::length(r);
    if (R < 1e-8f) return;
    glm::vec3 u = r / R;
    glm::vec3 F_on_a;
    if (qm_surface_h2_) {
        float dEdr = qm_surface_h2_->dEdr(R);
        F_on_a = dEdr * u;
    } else if (config.enable_dynamic_qm) {
        F_on_a = compute_qm_force(atoms[0], atoms[1]);
    } else {
        return;  // No PES or dynamic
    }
    
    //std::cout << "F_on_a before dampening: " << F_on_a << std::endl;
    // Damping (existing)
    if (config.enable_qm_bond_damping) {
        glm::vec3 v_rel = b.velocity - a.velocity;
        float v_rel_rad = glm::dot(v_rel, u);
        float gamma = config.qm_bond_damping_gamma;
        if (gamma > 0.0f) {
            float mu = (a.mass * b.mass) / (a.mass + b.mass);
            float f_damp = -mu * gamma * v_rel_rad;
            F_on_a += (f_damp * u);
        }
    }
    //std::cout << "F_on_a after dampening: " << F_on_a << std::endl;
    glm::vec3 F_on_b = -F_on_a;
    a.velocity += (F_on_a / a.mass) * dt;
    b.velocity += (F_on_b / b.mass) * dt;
    //std::cout << "Exiting applyQmSurfaceH2Forces" << std::endl;
}

void Simulation::applyQmForces(float dt) {
    if (atoms.size() != 2) return;  // For now; generalize later
    if (dt <= 0.0f) return;
    auto& a = atoms[0].nucleus;
    auto& b = atoms[1].nucleus;
    glm::vec3 r = b.position - a.position;
    float R = glm::length(r);
    if (R < 1e-8f) return;
    glm::vec3 u = r / R;
    glm::vec3 F_on_a;
    if (qm_surface_h2_) {
        float dEdr = qm_surface_h2_->dEdr(R);
        F_on_a = dEdr * u;
    } else if (config.enable_dynamic_qm) {
        F_on_a = compute_qm_force(atoms[0], atoms[1]);
    } else {
        return;
    }
    // Damping
    if (config.enable_qm_bond_damping) {
        glm::vec3 v_rel = b.velocity - a.velocity;
        float v_rel_rad = glm::dot(v_rel, u);
        float gamma = config.qm_bond_damping_gamma;
        if (gamma > 0.0f) {
            float mu = (a.mass * b.mass) / (a.mass + b.mass);
            float f_damp = mu * gamma * v_rel_rad;
            F_on_a += (f_damp * u);  // opposes motion
        }
    }

    glm::vec3 F_on_b = -F_on_a;
    a.velocity += (F_on_a / a.mass) * dt;
    b.velocity += (F_on_b / b.mass) * dt;
}


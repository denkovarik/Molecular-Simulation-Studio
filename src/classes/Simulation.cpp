// src/classes/Simulation.cpp
#include "Simulation.hpp"

#include <random>
#include <cmath>
#include <stdexcept>
#include <string>
#include <algorithm>    // std::max

#include <glm/glm.hpp>

#include "../physics/Coulomb.hpp"

// Constructor: Set up container and generate particles
Simulation::Simulation(const Config& cfg)
    : container(cfg.containerMaxX, cfg.containerMinX,
                cfg.containerMaxY, cfg.containerMinY,
                cfg.containerMaxZ, cfg.containerMinZ),
      config(cfg)
{
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

    for (int n = 0; n < cfg.numParticles; ++n) {
        bool placed = false;

        for (int attempt = 0; attempt < cfg.maxPlacementAttemptsPerParticle; ++attempt) {
            glm::vec3 pos(x_dist(gen), y_dist(gen), z_dist(gen));
            glm::vec3 vel(vel_dist(gen), vel_dist(gen), vel_dist(gen));
            Particle newParticle(cfg.particleMass, cfg.particleRadius, pos, vel);

            bool collides = false;
            for (const auto& p : container.particles) {
                if (container.particlesCollide(newParticle, p)) {
                    collides = true;
                    break;
                }
            }

            if (!collides) {
                container.particles.push_back(newParticle);
                placed = true;
                break;
            }
        }

        if (!placed) {
            throw std::runtime_error(
                "Simulation init: failed to place particle " + std::to_string(n) +
                " without overlap. Reduce numParticles, reduce radius, or enlarge container."
            );
        }
    }
}

void Simulation::computeChemicalForces(float dt) {
    auto& ps = container.particles;

    // Guard against silly dt (helps tests, too)
    if (dt <= 0.0f) return;

    for (size_t i = 0; i < ps.size(); ++i) {
        for (size_t j = i + 1; j < ps.size(); ++j) {
            Particle& a = ps[i];
            Particle& b = ps[j];

            glm::vec3 r = b.position - a.position; // a -> b
            float r2 = glm::dot(r, r);

            // Avoid divide-by-zero / huge forces
            if (r2 < 1e-12f) continue;

            float dist = std::sqrt(r2);

            // Lennard-Jones terms
            float sr   = config.lj_sigma / dist;
            float sr2  = sr * sr;
            float sr6  = sr2 * sr2 * sr2;
            float sr12 = sr6 * sr6;

            // Force on a: F = 24ε/r * (2(σ/r)^12 - (σ/r)^6) * rhat
            // => F = 24ε * (2sr12 - sr6) / r^2 * rvec
            float f_over_r = 24.0f * config.lj_epsilon * (2.0f * sr12 - sr6) / (dist * dist);

            glm::vec3 F = f_over_r * r; // force ON a

            a.velocity += (F / a.mass) * dt;
            b.velocity -= (F / b.mass) * dt;
        }
    }
}

void Simulation::computeCoulombForces(float dt) {
    if (!config.enable_coulomb) return;
    if (dt <= 0.0f) return;

    auto& ps = container.particles;

    // Softening must be > 0 to avoid singularities if someone sets it to 0
    const float soft = std::max(config.coulomb_softening, 1e-6f);

    for (size_t i = 0; i < ps.size(); ++i) {
        for (size_t j = i + 1; j < ps.size(); ++j) {
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

void Simulation::update(float dt) {
    // Forces (can coexist)
    if (config.enable_chemistry) {
        computeChemicalForces(dt);  // LJ
    }
    computeCoulombForces(dt);       // Coulomb (separate flag)

    // Spatial / collisions
    container.assignParticles2Grid();

    if (!config.enable_chemistry) {
        // Keep your old "bouncing balls" behavior when chemistry is off
        container.resolveParticleCollisions();
    }

    container.checkWallCollisions();

    // Integrate positions
    for (auto& p : container.particles) {
        if (config.enable_chemistry) {
            // Small damping to keep the chemistry demo stable (optional)
            p.velocity *= 0.995f;
        }
        p.position += p.velocity * dt;
    }
}


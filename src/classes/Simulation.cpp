// src/classes/Simulation.cpp
#include "Simulation.hpp"
#include <random>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp> // For glm::to_string
#include <iostream> // For std::cout
// Constructor: Set up container and generate particles
Simulation::Simulation(const Config& cfg)
    : container(cfg.containerMaxX, cfg.containerMinX,
                cfg.containerMaxY, cfg.containerMinY,
                cfg.containerMaxZ, cfg.containerMinZ),
                config(cfg)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    while (container.particles.size() < static_cast<size_t>(cfg.numParticles)) {
        std::uniform_real_distribution<float> x_dist(cfg.containerMinX + cfg.particleRadius * 2,
                                                     cfg.containerMaxX - cfg.particleRadius * 2);
        std::uniform_real_distribution<float> y_dist(cfg.containerMinY + cfg.particleRadius * 2,
                                                     cfg.containerMaxY - cfg.particleRadius * 2);
        std::uniform_real_distribution<float> z_dist(cfg.containerMinZ + cfg.particleRadius * 2,
                                                     cfg.containerMaxZ - cfg.particleRadius * 2);
        std::uniform_real_distribution<float> vel_dist(-cfg.velocityRange, cfg.velocityRange);
        bool placed = false;
        while (!placed) {
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
            }
        }
    }
}
void Simulation::computeChemicalForces(float dt) { // <-- Pass dt here
    for (size_t i = 0; i < container.particles.size(); ++i) {
        for (size_t j = i + 1; j < container.particles.size(); ++j) {
            Particle& p1 = container.particles[i];
            Particle& p2 = container.particles[j];
            glm::vec3 r = p2.position - p1.position;
            float dist = glm::length(r);
            if (dist < 1e-6f) continue; // avoid div-by-zero
            float sigma_over_r = config.lj_sigma / dist;
            float sor6 = std::pow(sigma_over_r, 6.0f);
            float sor12 = sor6 * sor6;
            // 24ε [2(σ/r)^12 - (σ/r)^6] / r (standard LJ force)
            float force_mag = 24.0f * config.lj_epsilon / dist *
                              (2.0f * sor12 - sor6);
            glm::vec3 force = -force_mag * glm::normalize(r);
            // Apply as acceleration: Δv = F/m * dt
            p1.velocity += (force / p1.mass) * dt;
            p2.velocity += (-force / p2.mass) * dt;
          
            // DEBUG LINE — comment out later
            if (dist < config.lj_sigma * 1.5f)
                std::cout << "dist=" << dist
                          << " force=" << force_mag
                          << " pos0=" << glm::to_string(p1.position)
                          << " pos1=" << glm::to_string(p2.position) << '\n';
        }
    }
}
void Simulation::update(float dt) {
    if (config.enable_chemistry) {
        computeChemicalForces(dt); // <-- Pass dt
    }
    container.assignParticles2Grid();
    if (!config.enable_chemistry) {
        container.resolveParticleCollisions(); // only for bouncing balls
    }
    container.checkWallCollisions(); // keep walls in both modes
    for (auto& p : container.particles) {
        if (config.enable_chemistry) {
            p.velocity *= 0.995f; // Add damping for stability
        }
        p.position += p.velocity * dt; // Simple Euler; update position with dt
    }
}

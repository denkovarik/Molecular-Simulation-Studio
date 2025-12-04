// src/classes/Simulation.cpp
#include "Simulation.hpp"
#include <random>

// Constructor: Set up container and generate particles
Simulation::Simulation(const Config& cfg)
    : container(cfg.containerMaxX, cfg.containerMinX,
                cfg.containerMaxY, cfg.containerMinY,
                cfg.containerMaxZ, cfg.containerMinZ)
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

// Update: Step the simulation physics
void Simulation::update(float dt) {
    container.assignParticles2Grid();
    container.resolveParticleCollisions();
    container.checkWallCollisions();
    for (auto& p : container.particles) {
        p.updatePosition(dt);
    }
}

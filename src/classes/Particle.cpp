#include "Particle.hpp"
#include <glm/glm.hpp>

Particle::Particle(glm::vec3 pos, glm::vec3 vel) : position(pos), velocity(vel) {}
Particle::Particle(float mass, float radius, glm::vec3 pos, glm::vec3 vel) : mass(mass), radius(radius), position(pos), velocity(vel) {}

void Particle::applyForce(glm::vec3& force) {
    if(force == glm::vec3(0.0f, 0.0f, 0.0f)) {
        return;
    }
    
    // Update velocity
    glm::vec3 acceleration = force / mass; // Calc acceleration
    // v = v_initial + a * t
    velocity = velocity + acceleration;  // t assumed to always be 1
};

void Particle::updatePosition() {
    position += velocity; // Assuming Δt is always 1
}
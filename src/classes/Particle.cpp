// src/classes/Particle.cpp

#include "Particle.hpp"

Particle::Particle(glm::vec3 pos, glm::vec3 vel) : position(pos), velocity(vel) {}

Particle::Particle(float mass, float radius, glm::vec3 pos, glm::vec3 vel) 
    : mass(mass), radius(radius), position(pos), velocity(vel) {}
    
Particle::Particle(float mass, float radius, float charge, glm::vec3 pos, glm::vec3 vel)
    : mass(mass), radius(radius), charge(charge), position(pos), velocity(vel) {}


void Particle::applyImpulse(glm::vec3 impulse) 
{
    if (impulse == glm::vec3(0.0f)) return;
    velocity += impulse / mass;
}

void Particle::update(float dt) {
    glm::vec3 new_pos = position * 2.0f - prev_position + acceleration * dt * dt;  // Verlet
    velocity = (new_pos - position) / dt;
    prev_position = position;
    position = new_pos;
    acceleration = glm::vec3(0.0f);  // Reset after force accum
}

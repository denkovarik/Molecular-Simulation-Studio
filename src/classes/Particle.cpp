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

void Particle::updatePosition(float dt) 
{
    position += velocity * dt;
}

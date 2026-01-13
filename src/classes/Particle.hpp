// src/classes/Particle.hpp

#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include <glm/glm.hpp>

class Particle 
{
public:
    float mass   = 1.0f;
    float radius = 0.1f;
    float charge = 0.0f;

    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};

    Particle(glm::vec3 pos, glm::vec3 vel);
    Particle(float mass, float radius, glm::vec3 pos, glm::vec3 vel);
    Particle(float mass, float radius, float charge, glm::vec3 pos, glm::vec3 vel);

    void applyImpulse(glm::vec3 impulse);
    void updatePosition(float dt);
};

#endif // PARTICLE_HPP


// src/classes/Particle.hpp (updated updatePosition to take dt)
#ifndef PARTICLE_HPP
#define PARTICLE_HPP
#include <glm/glm.hpp>

class Particle {
public:
    float mass = 1.0f;
    float radius = 0.1f;
    glm::vec3 position;
    glm::vec3 velocity;

    Particle(glm::vec3 pos, glm::vec3 vel);
    Particle(float mass, float radius, glm::vec3 pos, glm::vec3 vel);

    void applyImpulse(glm::vec3 impulse);  // Assuming you have this from earlier
    void updatePosition(float dt);  // Now takes dt
};

#endif // PARTICLE_HPP

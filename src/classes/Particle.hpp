#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include <vector>
#include <glm/glm.hpp>

class Container;

class Particle {
public:
    glm::vec3 velocity;
    glm::vec3 position;
    float radius = 0.1f;
    float mass = 1.0f;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    Particle(glm::vec3 pos, glm::vec3 vel);
    Particle(float mass, float radius, glm::vec3 pos, glm::vec3 vel);
    
    void applyForce(glm::vec3& force);
    void updatePosition();
};

# endif // PARTICLE_HPP
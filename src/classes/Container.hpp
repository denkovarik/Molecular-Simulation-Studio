#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include <glm/glm.hpp>
#include "Particle.hpp"
#include <vector>

class Container {
public:
    float rightWallX = 2.0f;
    float leftWallX = -2.0f;
    float ceilingY = 2.0f;
    float floorY = -2.0f;
    float frontWallZ = 2.0f;
    float backWallZ = -2.0f;
    std::vector<Particle> particles;
    
    void checkWallCollisions();
};

#endif // CONTAINER_HPP
#include "Container.hpp"

void Container::checkWallCollisions() {
    for(int i = 0; i < particles.size(); i++) {
        glm::vec3 force = glm::vec3(0.0f, 0.0f, 0.0f);
        
        // X-axis collision
        if (particles[i].position.x + particles[i].radius > rightWallX) {
            particles[i].position.x = rightWallX - particles[i].radius;
            force.x = particles[i].mass * -2.0 * particles[i].velocity.x;
        } else if (particles[i].position.x - particles[i].radius < leftWallX) {
            particles[i].position.x = leftWallX + particles[i].radius;
            force.x = particles[i].mass * -2.0 * particles[i].velocity.x;
        }

        // Y-axis collision
        if (particles[i].position.y + particles[i].radius > ceilingY) {
            particles[i].position.y = ceilingY - particles[i].radius;
            force.y = particles[i].mass * -2.0 * particles[i].velocity.y;
        } else if (particles[i].position.y - particles[i].radius < floorY) {
            particles[i].position.y = floorY + particles[i].radius;
            force.y = particles[i].mass * -2.0 * particles[i].velocity.y;
        }

        // Z-axis collision
        if (particles[i].position.z + particles[i].radius > frontWallZ) {
            particles[i].position.z = frontWallZ - particles[i].radius;
            force.z = particles[i].mass * -2.0 * particles[i].velocity.z;
        } else if (particles[i].position.z - particles[i].radius < backWallZ) {
            particles[i].position.z = backWallZ + particles[i].radius;
            force.z = particles[i].mass * -2.0 * particles[i].velocity.z;
        }   
        
        particles[i].applyForce(force);
    }
}
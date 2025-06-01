#include "Container.hpp"


bool Container::particlesCollide(Particle& a, Particle& b) {
    // Vector between centers
    glm::vec3 r = b.position - a.position;
    float distance = glm::length(r);
    float sumRadii = a.radius + b.radius;

    // Skip if particles are not overlapping
    if (distance < sumRadii) {
        return true;
    }
    
    return false;
}

void Container::resolveParticleCollisions() {
    for (int i = 0; i < particles.size(); ++i) {
        for (int j = i + 1; j < particles.size(); ++j) {
            Particle& a = particles[i];
            Particle& b = particles[j];

            // Vector between centers
            glm::vec3 r = b.position - a.position;

            // Skip if particles are not overlapping
            if (particlesCollide(a, b)) {
                // Normalize the collision normal
                glm::vec3 n = glm::normalize(r);

                // Relative velocity
                glm::vec3 v_rel = b.velocity - a.velocity;
                float v_rel_n = glm::dot(v_rel, n);

                // Only resolve if particles are moving toward each other
                if (v_rel_n < 0.0f) {
                    float e = 1.0f; // Elastic collision
                    float invMassA = 1.0f / a.mass;
                    float invMassB = 1.0f / b.mass;

                    // Compute impulse
                    float J = (1.0f + e) * v_rel_n / (invMassA + invMassB);

                    // Apply impulse to velocities
                    glm::vec3 impulseA = J * n;
                    glm::vec3 impulseB = -J * n;

                    a.applyForce(impulseA);
                    b.applyForce(impulseB);
                }
            }
        }
    }
}

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
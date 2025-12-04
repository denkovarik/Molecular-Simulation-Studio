// src/classes/Container.cpp 
#include "Container.hpp"
#include <iostream>
#include <cmath>
#include <set>


Cell::Cell(float maxX, float minX, float maxY, float minY, float maxZ, float minZ) {
    xMin = minX;
    xMax = maxX;
    yMin = minY;
    yMax = maxY;
    zMin = minZ;
    zMax = maxZ;
    
    
}

Container::Container() {
    constructGrid();
}

Container::Container(float maxX, float minX, float maxY, float minY, float maxZ, float minZ) {
    rightWallX = maxX;
    leftWallX = minX;
    ceilingY = maxY;
    floorY = minY;
    frontWallZ = maxZ;
    backWallZ = minZ;
    
    constructGrid();
}

void Container::constructGrid() {
    float cellAxisLen = 0.3f;

    theGrid = Grid();
    
    float xPos = leftWallX;
    for(int i = 0; xPos < rightWallX; i++) {
        theGrid.theMatrix.push_back(std::vector<std::vector<Cell>>());
        float yPos = floorY;
        for(int j = 0; yPos < ceilingY; j++) {
            theGrid.theMatrix[i].push_back(std::vector<Cell>());
            float zPos = backWallZ;
            for(int p = 0; zPos < frontWallZ; p++) {
                float cellXMax = xPos + cellAxisLen;
                float cellYMax = yPos + cellAxisLen;
                float cellZMax = zPos + cellAxisLen;
                Cell newCell = Cell(xPos, cellXMax, yPos, cellYMax, zPos, cellZMax);
                theGrid.theMatrix[i][j].push_back(newCell);
                
                zPos += cellAxisLen;
            }
            yPos += cellAxisLen;
        }
        xPos += cellAxisLen;
    }
};

void Container::assignParticles2Grid() {
    clearGrid();
    
    for(int i = 0; i < particles.size(); i++) {        
        std::vector<int> gridIndex = computeGridIndex(particles[i]);
        
        theGrid.theMatrix[gridIndex[0]][gridIndex[1]][gridIndex[2]].particleIndies.push_back(i);
    }
}

std::vector<int> Container::computeGridIndex(Particle& particle) {
    int x = (fabs(leftWallX) + particle.position.x) / theGrid.cell_axis_len;
    int y = (fabs(floorY) + particle.position.y) / theGrid.cell_axis_len;
    int z = (fabs(backWallZ) + particle.position.z) / theGrid.cell_axis_len;
    
    if(x < 0) {
        x = 0;
    } else if(x >= theGrid.theMatrix.size()) {
        x = theGrid.theMatrix.size() - 1;
    }

    if(y < 0) {
        y = 0;
    } else if(y >= theGrid.theMatrix[0].size()) {
        y = theGrid.theMatrix[0].size() - 1;
    }
    
    if(z < 0) {
        z = 0;
    } else if(z >= theGrid.theMatrix[0][0].size()) {
        z = theGrid.theMatrix[0][0].size() - 1;
    }
    
    std::vector<int> ind{x, y, z};
    return ind;
}

void Container::clearGrid() {
    for(int i = 0; i < theGrid.theMatrix.size(); i++) {
        for(int j = 0; j < theGrid.theMatrix[i].size(); j++) {
            for(int p = 0; p < theGrid.theMatrix[i][j].size(); p++) {
                theGrid.theMatrix[i][j][p].particleIndies.clear();
            }        
        }
    }
}

bool Container::particlesCollide(const Particle& a, const Particle& b) {
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

void Container::resolveParticleCollision(Particle& a, Particle& b) {
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

            glm::vec3 impulseA = J * n;
            glm::vec3 impulseB = -J * n;
            a.applyImpulse(impulseA);
            b.applyImpulse(impulseB);
        }
    }
}

void Container::resolveParticleCollisions() {
    std::set<std::pair<int, int>> collisions;
    for (int i = 0; i < particles.size(); ++i) {
        std::vector<int> gridIndex = computeGridIndex(particles[i]);
        
        int xMin = theGrid.getGridXMin(gridIndex[0], 1);
        int xMax = theGrid.getGridXMax(gridIndex[0], 1);
        int yMin = theGrid.getGridYMin(gridIndex[1], 1);
        int yMax = theGrid.getGridYMax(gridIndex[1], 1);
        int zMin = theGrid.getGridZMin(gridIndex[2], 1);
        int zMax = theGrid.getGridZMax(gridIndex[2], 1);
        
        for(int x = xMin; x < xMax; x++) {
            for(int y = yMin; y < yMax; y++) {
                for(int z = zMin; z < zMax; z++) {
                    for(int p = 0; p < theGrid.theMatrix[x][y][z].particleIndies.size(); p++) {
                        int j = theGrid.theMatrix[x][y][z].particleIndies[p];
                        if(particlesCollide(particles[i], particles[j])) {
                            std::pair<int, int> collision = std::pair<int, int>(std::min(i, j), std::max(i, j));
                            if(collisions.find(collision) == collisions.end()) {
                                resolveParticleCollision(particles[i], particles[j]);
                                collisions.insert(collision);
                            }
                        }
                    }
                }
            }   
        } 
    }
}

int Grid::getGridXMin(int x, int range) {
    int xMin = x - 1;
    if(xMin < 0) {
        xMin = 0;
    }
    return xMin;
}

int Grid::getGridXMax(int x, int range) {
    int xMax = x + 1;
    if(xMax >= theMatrix.size()) {
        xMax = x;
    }
    return xMax;
}

int Grid::getGridYMin(int y, int range) {
    int yMin = y - 1;
    if(yMin < 0) {
        yMin = 0;
    }
    return yMin;
}

int Grid::getGridYMax(int y, int range) {
    int yMax = y + 1;
    if(yMax >= theMatrix[0].size()) {
        yMax = y;
    }
    return yMax;
}

int Grid::getGridZMin(int z, int range) {
    int zMin = z - 1;
    if(zMin < 0) {
        zMin = 0;
    }
    return zMin;
}

int Grid::getGridZMax(int z, int range) {
    int zMax = z + 1;
    if(zMax >= theMatrix[0][0].size()) {
        zMax = z;
    }
    return zMax;
}

void Container::checkWallCollisions() {
    for(int i = 0; i < particles.size(); i++) {
        glm::vec3 impulse(0.0f, 0.0f, 0.0f);
        
        // X-axis collision
        if (particles[i].position.x + particles[i].radius > rightWallX) {
            particles[i].position.x = rightWallX - particles[i].radius;
            impulse.x = particles[i].mass * -2.0 * particles[i].velocity.x;
        } else if (particles[i].position.x - particles[i].radius < leftWallX) {
            particles[i].position.x = leftWallX + particles[i].radius;
            impulse.x = particles[i].mass * -2.0 * particles[i].velocity.x;
        }

        // Y-axis collision
        if (particles[i].position.y + particles[i].radius > ceilingY) {
            particles[i].position.y = ceilingY - particles[i].radius;
            impulse.y = particles[i].mass * -2.0 * particles[i].velocity.y;
        } else if (particles[i].position.y - particles[i].radius < floorY) {
            particles[i].position.y = floorY + particles[i].radius;
            impulse.y = particles[i].mass * -2.0 * particles[i].velocity.y;
        }

        // Z-axis collision
        if (particles[i].position.z + particles[i].radius > frontWallZ) {
            particles[i].position.z = frontWallZ - particles[i].radius;
            impulse.z = particles[i].mass * -2.0 * particles[i].velocity.z;
        } else if (particles[i].position.z - particles[i].radius < backWallZ) {
            particles[i].position.z = backWallZ + particles[i].radius;
            impulse.z = particles[i].mass * -2.0 * particles[i].velocity.z;
        }   
        
        particles[i].applyImpulse(impulse);
    }
}

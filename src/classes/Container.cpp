// src/classes/Container.cpp 
#include "Container.hpp"
#include <iostream>
#include <cmath>
#include <set>


Cell::Cell(float minX, float maxX, float minY, float maxY, float minZ, float maxZ) {
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
    
    for (size_t i = 0; i < particles.size(); ++i) {
        std::vector<size_t> gridIndex = computeGridIndex(particles[i]);
        
        theGrid.theMatrix[gridIndex[0]][gridIndex[1]][gridIndex[2]].particleIndies.push_back(i);
    }
}

std::vector<size_t> Container::computeGridIndex(Particle& particle) {
    size_t x = static_cast<size_t>((fabs(leftWallX) + particle.position.x) / theGrid.cell_axis_len);
    size_t y = static_cast<size_t>((fabs(floorY) + particle.position.y) / theGrid.cell_axis_len);
    size_t z = static_cast<size_t>((fabs(backWallZ) + particle.position.z) / theGrid.cell_axis_len);
    
    if (x >= theGrid.theMatrix.size()) {
        x = theGrid.theMatrix.size() - 1;
    }

    if (y >= theGrid.theMatrix[0].size()) {
        y = theGrid.theMatrix[0].size() - 1;
    }
    
    if (z >= theGrid.theMatrix[0][0].size()) {
        z = theGrid.theMatrix[0][0].size() - 1;
    }
    
    std::vector<size_t> ind{x, y, z};
    return ind;
}

void Container::clearGrid() {
    for(int i = 0; (size_t)i < theGrid.theMatrix.size(); i++) {
        for(int j = 0; (size_t)j < theGrid.theMatrix[i].size(); j++) {
            for(int p = 0; (size_t)p < theGrid.theMatrix[i][j].size(); p++) {
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
    // Precompute all 27 possible offsets for 3x3x3 neighborhood
    const int offsets[27][3] = {
        {-1, -1, -1}, {-1, -1, 0}, {-1, -1, 1},
        {-1,  0, -1}, {-1,  0, 0}, {-1,  0, 1},
        {-1,  1, -1}, {-1,  1, 0}, {-1,  1, 1},
        { 0, -1, -1}, { 0, -1, 0}, { 0, -1, 1},
        { 0,  0, -1}, { 0,  0, 0}, { 0,  0, 1},
        { 0,  1, -1}, { 0,  1, 0}, { 0,  1, 1},
        { 1, -1, -1}, { 1, -1, 0}, { 1, -1, 1},
        { 1,  0, -1}, { 1,  0, 0}, { 1,  0, 1},
        { 1,  1, -1}, { 1,  1, 0}, { 1,  1, 1}
    };

    const size_t numOffsets = 27;
    const size_t gridXSize = theGrid.theMatrix.size();
    const size_t gridYSize = (gridXSize > 0) ? theGrid.theMatrix[0].size() : 0;
    const size_t gridZSize = (gridYSize > 0) ? theGrid.theMatrix[0][0].size() : 0;

    for (size_t i = 0; i < particles.size(); ++i) {
        std::vector<size_t> gridIndex = computeGridIndex(particles[i]);

        // Single loop over fixed offsets instead of 3 nested dimensional loops
        for (size_t k = 0; k < numOffsets; ++k) {
            int dx = offsets[k][0];
            int dy = offsets[k][1];
            int dz = offsets[k][2];

            // Compute neighbor indices with overflow/underflow checks
            // Use signed int for calculation to handle negative offsets safely
            ptrdiff_t nx = static_cast<ptrdiff_t>(gridIndex[0]) + dx;
            ptrdiff_t ny = static_cast<ptrdiff_t>(gridIndex[1]) + dy;
            ptrdiff_t nz = static_cast<ptrdiff_t>(gridIndex[2]) + dz;

            // Skip invalid cells
            if (nx < 0 || nx >= static_cast<ptrdiff_t>(gridXSize) ||
                ny < 0 || ny >= static_cast<ptrdiff_t>(gridYSize) ||
                nz < 0 || nz >= static_cast<ptrdiff_t>(gridZSize)) {
                continue;
            }

            const auto& cellIndies = theGrid.theMatrix[nx][ny][nz].particleIndies;

            // Inner loop over particles in the neighbor cell
            for (size_t p = 0; p < cellIndies.size(); ++p) {
                size_t j = cellIndies[p];
                if (j <= i) continue;  // Skip self and duplicates (ensures pairs are unique)

                if (particlesCollide(particles[i], particles[j])) {
                    resolveParticleCollision(particles[i], particles[j]);
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
    if((size_t)xMax >= theMatrix.size()) {
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
    if((size_t)yMax >= theMatrix[0].size()) {
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
    if((size_t)zMax >= theMatrix[0][0].size()) {
        zMax = z;
    }
    return zMax;
}

void Container::checkWallCollisions() {
    for(int i = 0; (size_t)i < particles.size(); i++) {
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

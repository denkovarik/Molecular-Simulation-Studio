#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include <glm/glm.hpp>
#include "Particle.hpp"
#include <vector>


class Cell {
public:
    float xMin = 0.0f;
    float xMax = 1.0f;
    float yMin = 0.0f;
    float yMax = 1.0f;
    float zMin = 0.0f;
    float zMax = 1.0f;
    std::vector<int> particleIndies;  // Indies of the particles 
    
    Cell(float maxX, float minX, float maxY, float minY, float maxZ, float minZ);
};


class Grid {
public:
    float cell_axis_len = 0.1;
    std::vector<std::vector<std::vector<Cell>>> theMatrix;
    
    int getGridXMin(int x, int range);
    int getGridXMax(int x, int range);
    int getGridYMin(int y, int range);
    int getGridYMax(int y, int range);
    int getGridZMin(int z, int range);
    int getGridZMax(int z, int range);
};


class Container {
public:
    float rightWallX = 2.0f;
    float leftWallX = -2.0f;
    float ceilingY = 2.0f;
    float floorY = -2.0f;
    float frontWallZ = 2.0f;
    float backWallZ = -2.0f;
    std::vector<Particle> particles;
    Grid theGrid;
    
    Container();
    Container(float maxX, float minX, float maxY, float minY, float maxZ, float minZ);
    
    void constructGrid();
    void assignParticles2Grid();
    std::vector<int> computeGridIndex(Particle& particle);
    void clearGrid();
    bool particlesCollide(Particle& a, Particle& b);
    void resolveParticleCollision(Particle& a, Particle& b);
    void resolveParticleCollisions();
    void checkWallCollisions();
};

#endif // CONTAINER_HPP
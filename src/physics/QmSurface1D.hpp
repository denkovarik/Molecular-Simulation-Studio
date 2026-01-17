// src/physics/QmSurface1D.hpp
#pragma once

#include <vector>

struct QmSample {
    float R; // distance
    float E; // energy
};

class QmSurface1D {
public:
    explicit QmSurface1D(const std::vector<QmSample>& samples);

    float dEdr(float R) const;
    float E(float R) const;   // <-- add

private:
    std::vector<QmSample> samples_;
};



// src/physics/QmSurface1D.cpp
#include "QmSurface1D.hpp"
#include <stdexcept>
#include <cmath>

static bool nearlyEqual(float a, float b, float eps = 1e-6f) {
    return std::fabs(a - b) < eps;
}

QmSurface1D::QmSurface1D(const std::vector<QmSample>& samples)
    : samples_(samples)
{
    if (samples_.size() < 2)
        throw std::runtime_error("QmSurface1D requires at least 2 samples");

    // Optional sanity: require increasing R
    for (size_t i = 1; i < samples_.size(); ++i) {
        if (!(samples_[i].R > samples_[i-1].R))
            throw std::runtime_error("QmSurface1D samples must have strictly increasing R");
    }
}

float QmSurface1D::dEdr(float R) const
{
    const size_t n = samples_.size();

    // If R matches a sample point, use finite differences (central if possible)
    for (size_t i = 0; i < n; ++i) {
        if (nearlyEqual(R, samples_[i].R)) {
            if (i == 0) {
                const auto& a = samples_[0];
                const auto& b = samples_[1];
                return (b.E - a.E) / (b.R - a.R);
            }
            if (i == n - 1) {
                const auto& a = samples_[n - 2];
                const auto& b = samples_[n - 1];
                return (b.E - a.E) / (b.R - a.R);
            }
            const auto& prev = samples_[i - 1];
            const auto& next = samples_[i + 1];
            return (next.E - prev.E) / (next.R - prev.R);
        }
    }

    // Clamp outside range using end slopes
    if (R <= samples_.front().R) {
        const auto& a = samples_[0];
        const auto& b = samples_[1];
        return (b.E - a.E) / (b.R - a.R);
    }
    if (R >= samples_.back().R) {
        const auto& a = samples_[n - 2];
        const auto& b = samples_[n - 1];
        return (b.E - a.E) / (b.R - a.R);
    }

    // Inside an interval: linear slope for now
    for (size_t i = 0; i + 1 < n; ++i) {
        const auto& a = samples_[i];
        const auto& b = samples_[i + 1];
        if (R > a.R && R < b.R) {
            return (b.E - a.E) / (b.R - a.R);
        }
    }

    return 0.0f; 
}

float QmSurface1D::E(float R) const
{
    const size_t n = samples_.size();

    // Exact sample match
    for (size_t i = 0; i < n; ++i) {
        if (nearlyEqual(R, samples_[i].R)) {
            return samples_[i].E;
        }
    }

    // Clamp outside range to endpoint energies
    if (R <= samples_.front().R) return samples_.front().E;
    if (R >= samples_.back().R)  return samples_.back().E;

    // Inside interval: linear interpolation
    for (size_t i = 0; i + 1 < n; ++i) {
        const auto& a = samples_[i];
        const auto& b = samples_[i + 1];

        if (R > a.R && R < b.R) {
            float t = (R - a.R) / (b.R - a.R);
            return a.E + t * (b.E - a.E);
        }
    }

    // Should be unreachable
    return samples_.back().E;
}


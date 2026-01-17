// src/physics/Coulomb.hpp

#pragma once

#include <glm/glm.hpp>
#include <cmath>

/*
  Softened Coulomb force.

  rvec  = position_b - position_a
  q1,q2 = charges
  k     = Coulomb constant (simulation units)
  soft  = softening length (prevents singularity at r → 0)

  Returns force ON particle a due to particle b.
*/
inline glm::vec3 coulombForce(
    const glm::vec3& rvec,
    float q1,
    float q2,
    float k,
    float soft
) 
{
    // Softened squared distance
    float r2 = glm::dot(rvec, rvec) + soft * soft;

    // Prevent NaNs if someone passes garbage
    if (r2 <= 0.0f) return glm::vec3(0.0f);

    // 1 / r³
    float inv_r = 1.0f / std::sqrt(r2);
    float inv_r3 = inv_r * inv_r * inv_r;

    // Coulomb force
    // Like charges → repulsion
    // Unlike charges → attraction
    return (k * q1 * q2) * (-rvec) * inv_r3;
}


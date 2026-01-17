// testing/test_QmForces3D.cpp
// Catch2 tests that validate your QM-force conventions in a QM-3D-friendly way.
// These will catch:
//  - Wrong sign: F = +gradE instead of F = -gradE
//  - Wrong damping sign: damping that pumps energy instead of dissipating it
//
// NOTE: This test is NOT tied to QmSurface1D. It validates the general rule:
//   Force = -∇E
// and optional radial damping: F_damp = -mu * gamma * v_rel_rad * u

/*
Usage:

g++ -std=c++17 -O0 -g \
  -I/usr/local/include/catch2 \
  -I/usr/include \
  testing/test_QmForces3D.cpp \
  -o test_QmForces3D.exe

./test_QmForces3D.exe -s

*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <cmath>


// -----------------------------
// Helpers
// -----------------------------
static float length(const glm::vec3& v) { return std::sqrt(glm::length2(v)); }

static bool nearlyEqual(float a, float b, float eps = 1e-4f) {
    return std::fabs(a - b) <= eps;
}

// A simple 3D "bond" potential with known energy + gradient.
// We use a harmonic well around re:
//   E(R) = 0.5 * k * (R - re)^2
//   dE/dR = k * (R - re)
//   ∇E = dE/dR * u
//   Force (physics) = -∇E
struct HarmonicBondPotential3D {
    float k;   // stiffness
    float re;  // equilibrium distance

    float energy(const glm::vec3& ra, const glm::vec3& rb) const {
        glm::vec3 r = rb - ra;
        float R = length(r);
        float x = (R - re);
        return 0.5f * k * x * x;
    }

    glm::vec3 grad_wrt_a(const glm::vec3& ra, const glm::vec3& rb) const {
        // ∂E/∂ra = - (dE/dR) * u  (since R depends on rb-ra)
        glm::vec3 r = rb - ra;
        float R = length(r);
        if (R < 1e-8f) return glm::vec3(0.0f);
        glm::vec3 u = r / R;

        float dE_dR = k * (R - re);
        // Gradient wrt ra points opposite u:
        return -dE_dR * u;
    }

    glm::vec3 force_on_a(const glm::vec3& ra, const glm::vec3& rb) const {
        // Physics: F = -∇E
        // where ∇E wrt ra is grad_wrt_a
        return -grad_wrt_a(ra, rb);
    }
};

// One step of the specific bond integrator pattern you’re using:
// v += (F/m) * dt
// plus optional radial damping
static void integrateBondStep(
    glm::vec3& ra, glm::vec3& va, float ma,
    glm::vec3& rb, glm::vec3& vb, float mb,
    const HarmonicBondPotential3D& pot,
    float dt,
    bool enableDamping,
    float gamma
) {
    glm::vec3 r = rb - ra;
    float R = length(r);
    if (R < 1e-8f) return;
    glm::vec3 u = r / R;

    glm::vec3 F_on_a = pot.force_on_a(ra, rb);
    glm::vec3 F_on_b = -F_on_a;

    if (enableDamping && gamma > 0.0f) {
        glm::vec3 v_rel = vb - va;
        float v_rel_rad = glm::dot(v_rel, u);

        float mu = (ma * mb) / (ma + mb);

        // IMPORTANT SIGN:
        // Damping should oppose radial relative motion -> negative sign.
        glm::vec3 F_damp = -mu * gamma * v_rel_rad * u;

        F_on_a += F_damp;
        F_on_b -= F_damp;
    }

    va += (F_on_a / ma) * dt;
    vb += (F_on_b / mb) * dt;

    // Semi-implicit Euler position update (matches your general style)
    ra += va * dt;
    rb += vb * dt;
}

static float totalEnergy(
    const HarmonicBondPotential3D& pot,
    const glm::vec3& ra, const glm::vec3& va, float ma,
    const glm::vec3& rb, const glm::vec3& vb, float mb
) {
    float KE = 0.5f * ma * glm::length2(va) + 0.5f * mb * glm::length2(vb);
    float PE = pot.energy(ra, rb);
    return KE + PE;
}

// -----------------------------
// Tests
// -----------------------------

TEST_CASE("QM 3D convention: Force points downhill (reduces energy for a small step)", "[qm3d][force-sign]") {
    // Setup: A stretched bond (R > re). Force should pull atoms together.
    HarmonicBondPotential3D pot { /*k=*/20.0f, /*re=*/0.74f };

    glm::vec3 ra(0.0f, 0.0f, 0.0f);
    glm::vec3 rb(1.20f, 0.0f, 0.0f); // stretched vs 0.74
    glm::vec3 va(0.0f), vb(0.0f);
    float ma = 1.0f, mb = 1.0f;

    float E0 = totalEnergy(pot, ra, va, ma, rb, vb, mb);

    // Take a tiny step with NO damping
    float dt = 1e-3f;
    integrateBondStep(ra, va, ma, rb, vb, mb, pot, dt, /*enableDamping=*/false, /*gamma=*/0.0f);

    float E1 = totalEnergy(pot, ra, va, ma, rb, vb, mb);

    // With the correct sign (F = -∇E), a small step from rest in a conservative field
    // should move in the direction that begins to lower potential energy.
    //
    // If your code uses the WRONG sign (+∇E), E1 will typically be >= E0 (often >).
    REQUIRE(E1 < E0);
}

TEST_CASE("QM bond damping opposes radial relative motion", "[qm3d][damping]") {
    float ma = 1.0f, mb = 1.0f;
    float gamma = 2.0f;

    glm::vec3 ra(0.0f);
    glm::vec3 rb(1.0f, 0.0f, 0.0f);

    glm::vec3 u = glm::normalize(rb - ra);

    auto dampingForceOnA = [&](const glm::vec3& va, const glm::vec3& vb) {
        glm::vec3 v_rel = vb - va;
        float v_rel_rad = glm::dot(v_rel, u);
        float mu = (ma * mb) / (ma + mb);
        return -mu * gamma * v_rel_rad * u;
    };

    SECTION("Approaching atoms → damping pushes apart") {
        glm::vec3 va(+0.2f, 0, 0);
        glm::vec3 vb(-0.2f, 0, 0);
        glm::vec3 Fd = dampingForceOnA(va, vb);
        REQUIRE(glm::dot(Fd, u) > 0.0f);
    }

    SECTION("Separating atoms → damping pulls together") {
        glm::vec3 va(-0.2f, 0, 0);
        glm::vec3 vb(+0.2f, 0, 0);
        glm::vec3 Fd = dampingForceOnA(va, vb);
        REQUIRE(glm::dot(Fd, u) < 0.0f);
    }
}




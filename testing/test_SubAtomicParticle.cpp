// testing/test_SubAtomicParticle.cpp
/*
Usage:

g++ -std=c++14 -I/usr/local/include -o test_SubAtomicParticle.exe \
    testing/test_SubAtomicParticle.cpp src/classes/SubAtomicParticle.cpp

./test_SubAtomicParticle.exe

*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "../src/classes/SubAtomicParticle.hpp"
#include <vector>
#include <cmath>

static void requireVecApprox(const std::vector<double>& a,
                             const std::vector<double>& b,
                             double eps = 1e-12) {
    REQUIRE(a.size() == b.size());
    for (size_t i = 0; i < a.size(); ++i) {
        REQUIRE(a[i] == Approx(b[i]).margin(eps));
    }
}

TEST_CASE("SubAtomicParticle constructor sets basic fields") {
    std::vector<double> position = {0.0, 0.0, 0.0};
    std::vector<double> velocity = {1.0, 2.0, 3.0};

    double mass   = 9.109e-31;   // electron mass (kg)
    double charge = -1.602e-19;  // electron charge (C)

    // Your class uses int spin; "1/2" would be 0, so pick an explicit int value.
    // For now we just test it round-trips.
    int spin = 1;

    SubAtomicParticle e(mass, position, velocity, charge, spin);

    REQUIRE(e.getMass() == Approx(mass));
    REQUIRE(e.getCharge() == Approx(charge));
    REQUIRE(e.getSpin() == spin);

    requireVecApprox(e.getPosition(), position);
    requireVecApprox(e.getVelocity(), velocity);

    // Forces are initialized to zero in your implementation
    requireVecApprox(e.getForces(), std::vector<double>{0.0, 0.0, 0.0});
}

TEST_CASE("SubAtomicParticle update integrates position with constant velocity") {
    std::vector<double> position = {0.0, 0.0, 0.0};
    std::vector<double> velocity = {1.0, 2.0, 3.0};

    double mass   = 1.6726e-27;  // proton mass (kg)
    double charge = 1.602e-19;   // proton charge (C)
    int spin      = 1;           // placeholder as int

    SubAtomicParticle p(mass, position, velocity, charge, spin);

    double dt = 1.0;
    p.update(dt);

    // Expected position = x0 + v*dt
    requireVecApprox(p.getPosition(), std::vector<double>{1.0, 2.0, 3.0}, 1e-12);
}


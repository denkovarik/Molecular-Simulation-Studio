// testing/test_QmSurfaceH2.cpp

/*
Build & run:

g++ -std=c++17 \
  -I/usr/local/include/catch2 \
  -I./src \
  testing/test_QmSurfaceH2.cpp \
  src/physics/QmSurface1D.cpp \
  -o test_QmSurfaceH2.exe

./test_QmSurfaceH2.exe
*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>

#include "../src/physics/QmSurface1D.hpp"

TEST_CASE("H2 QM surface produces correct force sign around equilibrium") {

    QmSurface1D surf({
        {0.50f,  5.0f},
        {0.74f,  0.0f},  
        {1.20f,  5.0f},  
        {2.00f,  8.0f}
    });

    glm::vec3 x1(0,0,0);

    SECTION("R > Re gives attraction") {
        float R = 1.10f;
        glm::vec3 x2(R, 0, 0);

        glm::vec3 u = glm::normalize(x2 - x1);

        // dE/dR is a scalar derivative w.r.t. the *distance* R.
        // Force on particle 1 is: F1 = + (dE/dR) * u
        // because ∂R/∂x1 = -u and F = -∇E.
        float dEdr = surf.dEdr(R);

        glm::vec3 F1 = dEdr * u;
        glm::vec3 F2 = -F1;

        REQUIRE(F1.x > 0.0f); // pulled toward other nucleus
        REQUIRE(F2.x < 0.0f);
    }

    SECTION("R < Re gives repulsion") {
        float R = 0.55f;
        glm::vec3 x2(R, 0, 0);

        glm::vec3 u = glm::normalize(x2 - x1);

        float dEdr = surf.dEdr(R);

        glm::vec3 F1 = dEdr * u;
        glm::vec3 F2 = -F1;

        REQUIRE(F1.x < 0.0f); // pushed away
        REQUIRE(F2.x > 0.0f);
    }

    SECTION("Near equilibrium force is near zero") {
        float dEdr = surf.dEdr(0.74f);
        REQUIRE(std::abs(dEdr) < 1e-2f);
    }

}


// testing/test_Atom.cpp
/*

Usage:

g++ -std=c++17 \
  -I/usr/local/include/catch2 \
  -I./src -I./src/classes \
  testing/test_Atom.cpp \
  src/classes/Atom.cpp \
  src/classes/Particle.cpp \
  -o test_Atom.exe

./test_Atom.exe

*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "../src/classes/Atom.hpp"
#include <glm/glm.hpp>

static void requireVecApprox(const std::vector<double>& a, const std::vector<double>& b, double eps = 1e-6) {
    REQUIRE(a.size() == b.size());
    for (size_t i = 0; i < a.size(); ++i) {
        REQUIRE(a[i] == Approx(b[i]).margin(eps));
    }
}

TEST_CASE("Atom constructor initializes nucleus and empty electrons") {
    std::string symbol = "H";
    int Z = 1;
    float mass = 1836.0f;  // Proton mass in AU
    float charge = 1.0f;
    float vdw_r = 0.53f;
    float h_k = 1.0f;
    float h_eq = 0.53f;
    glm::vec3 pos(0.0f, 0.0f, 0.0f);
    glm::vec3 vel(1.0f, 0.0f, 0.0f);

    Atom atom(symbol, Z, mass, charge, vdw_r, h_k, h_eq, pos, vel);

    // Check nucleus
    REQUIRE(atom.nucleus.mass == Approx(mass));
    REQUIRE(atom.nucleus.charge == Approx(charge));
    REQUIRE(atom.nucleus.radius == Approx(vdw_r / 2.0f));
    REQUIRE(atom.nucleus.position == pos);
    REQUIRE(atom.nucleus.velocity == vel);

    // Check atom properties
    REQUIRE(atom.elementSymbol == symbol);
    REQUIRE(atom.atomicNumber == Z);
    REQUIRE(atom.vanDerWaalsRadius == Approx(vdw_r));
    REQUIRE(atom.harmonic_k == Approx(h_k));
    REQUIRE(atom.harmonic_eq == Approx(h_eq));

    // Electrons empty initially
    REQUIRE(atom.electrons.empty());
}

TEST_CASE("Atom can add electrons with correct properties") {
    // Arrange: Create a basic H atom
    Atom h_atom("H", 1, 1836.0f, 1.0f, 0.53f, 1.0f, 0.53f, glm::vec3(0.0f), glm::vec3(0.0f));

    // Act: Add an electron (use SubAtomicParticle constructor)
    std::vector<double> e_pos = {0.53, 0.0, 0.0};  // Offset from nucleus
    std::vector<double> e_vel = {0.0, 1.0, 0.0};
    double e_mass = 1.0f;
    double e_charge = -1.0;
    int e_spin = 1;  // Up
    h_atom.electrons.emplace_back(e_mass, e_pos, e_vel, e_charge, e_spin);

    // Assert: Electron added correctly
    REQUIRE(h_atom.electrons.size() == 1);
    const auto& e = h_atom.electrons[0];
    REQUIRE(e.getMass() == Approx(e_mass));
    REQUIRE(e.getCharge() == Approx(e_charge));
    REQUIRE(e.getSpin() == e_spin);
    requireVecApprox(e.getPosition(), {0.53, 0.0, 0.0});
    requireVecApprox(e.getVelocity(), {0.0, 1.0, 0.0});
}

// Add near top if missing (adapted for glm::vec3)
static void requireVec3Approx(const glm::vec3& a, const glm::vec3& b, float margin = 1e-6f) {
    REQUIRE(a.x == Approx(b.x).margin(margin));
    REQUIRE(a.y == Approx(b.y).margin(margin));
    REQUIRE(a.z == Approx(b.z).margin(margin));
}

TEST_CASE("applyForces conserves total momentum for binding in isolated atom") {
    // Arrange: Config with electron mass
    Config cfg;
    cfg.electron_mass = 1.0f;  // AU

    // Create H atom with offset electron
    Atom h_atom("H", 1, 1836.0f, 1.0f, 0.53f, 1.0f, 0.53f, glm::vec3(0.0f), glm::vec3(0.0f));
    std::vector<double> e_pos = {1.0f, 0.0f, 0.0f};  // Displaced beyond eq
    std::vector<double> e_vel = {0.0f, 0.0f, 0.0f};
    h_atom.electrons.emplace_back(1.0f, e_pos, e_vel, -1.0f, 1);

    // Compute initial total momentum (should be zero)
    glm::vec3 p_nuc = h_atom.nucleus.mass * h_atom.nucleus.velocity;
    std::vector<double> e_v = h_atom.electrons[0].getVelocity();
    glm::vec3 p_e = cfg.electron_mass * glm::vec3(e_v[0], e_v[1], e_v[2]);
    glm::vec3 p_total_before = p_nuc + p_e;

    // Act: Apply forces (binding should pull, but conserve momentum)
    float dt = 0.001f;
    std::vector<Atom> others;  // Empty, isolated
    h_atom.applyForces(others, dt, cfg);

    // Assert: Total momentum unchanged 
    p_nuc = h_atom.nucleus.mass * h_atom.nucleus.velocity;
    e_v = h_atom.electrons[0].getVelocity();
    p_e = cfg.electron_mass * glm::vec3(e_v[0], e_v[1], e_v[2]);
    glm::vec3 p_total_after = p_nuc + p_e;

    requireVec3Approx(p_total_after, p_total_before, 1e-5f);

    // Bonus: Electron should accelerate toward nucleus
    REQUIRE(h_atom.electrons[0].getVelocity()[0] < 0.0);  // Pulled left
}

TEST_CASE("applyForces applies Coulomb repulsion between two atoms and conserves momentum") {
    // Arrange: Config with Coulomb params
    Config cfg;
    cfg.coulomb_k = 10.0f;
    cfg.coulomb_softening = 0.1f;

    // Two H atoms 
    std::vector<Atom> atoms;
    atoms.emplace_back("H", 1, 1836.0f, 1.0f, 0.53f, 1.0f, 0.53f, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f));
    atoms.emplace_back("H", 1, 1836.0f, 1.0f, 0.53f, 1.0f, 0.53f, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f));

    // Initial momentum (zero)
    glm::vec3 p_total_before = atoms[0].nucleus.mass * atoms[0].nucleus.velocity +
                               atoms[1].nucleus.mass * atoms[1].nucleus.velocity;

    // Act: Apply forces (Coulomb repulsion)
    float dt = 0.001f;
    atoms[0].applyForces(atoms, dt, cfg);
    atoms[1].applyForces(atoms, dt, cfg);

    // Assert: Momentum conserved
    glm::vec3 p_total_after = atoms[0].nucleus.mass * atoms[0].nucleus.velocity +
                              atoms[1].nucleus.mass * atoms[1].nucleus.velocity;
    requireVec3Approx(p_total_after, p_total_before, 1e-5f);

    // Repulsion: Left atom moves more left, right more right
    REQUIRE(atoms[0].nucleus.velocity.x < 0.0f);
    REQUIRE(atoms[1].nucleus.velocity.x > 0.0f);

    // Symmetry (equal masses)
    REQUIRE(std::abs(atoms[0].nucleus.velocity.x) == Approx(std::abs(atoms[1].nucleus.velocity.x)).margin(1e-6f));
}

TEST_CASE("applyForces applies Coulomb attraction between electron of one atom and nucleus of another") {
    // Arrange: Config with Coulomb and electron mass
    Config cfg;
    cfg.coulomb_k = 10.0f;
    cfg.coulomb_softening = 0.1f;
    cfg.electron_mass = 1.0f;

    // Two H atoms: Left with electron, right without (toy ionic setup)
    std::vector<Atom> atoms;
    atoms.emplace_back("H", 1, 1836.0f, 1.0f, 0.53f, 1.0f, 0.53f, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f));
    std::vector<double> e_pos = {-0.5f, 0.0f, 0.0f};  // Electron between, but bound to left
    std::vector<double> e_vel = {0.0f, 0.0f, 0.0f};
    atoms[0].electrons.emplace_back(1.0f, e_pos, e_vel, -1.0f, 1);
    atoms.emplace_back("H", 1, 1836.0f, 1.0f, 0.53f, 1.0f, 0.53f, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f));

    // Initial total momentum (zero)
    glm::vec3 p_total_before(0.0f);
    for (const auto& atom : atoms) {
        p_total_before += atom.nucleus.mass * atom.nucleus.velocity;
        for (const auto& e : atom.electrons) {
            std::vector<double> e_v = e.getVelocity();
            p_total_before += cfg.electron_mass * glm::vec3(e_v[0], e_v[1], e_v[2]);
        }
    }

    // Act: Apply forces (should attract electron to right nucleus)
    float dt = 0.001f;
    for (auto& atom : atoms) {
        atom.applyForces(atoms, dt, cfg);
    }

    // Assert: Momentum conserved
    glm::vec3 p_total_after(0.0f);
    for (const auto& atom : atoms) {
        p_total_after += atom.nucleus.mass * atom.nucleus.velocity;
        for (const auto& e : atom.electrons) {
            std::vector<double> e_v = e.getVelocity();
            p_total_after += cfg.electron_mass * glm::vec3(e_v[0], e_v[1], e_v[2]);
        }
    }
    requireVec3Approx(p_total_after, p_total_before, 1e-5f);

    // Attraction: Electron (on left) should accelerate right (toward right nucleus)
    std::vector<double> e_v_after = atoms[0].electrons[0].getVelocity();
    REQUIRE(e_v_after[0] > 0.0f);  // Positive x velocity change

    // Right nucleus should feel slight pull left (equal-opposite, scaled by mass)
    REQUIRE(atoms[1].nucleus.velocity.x < 0.0f);
}

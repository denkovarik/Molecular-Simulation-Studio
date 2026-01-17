// testing/test_Simulation.cpp
/*
Usage:

g++ -std=c++17 \
  -I/usr/local/include/catch2 \
  -I./src -I./src/classes \
  testing/test_Simulation.cpp \
  src/classes/Simulation.cpp \
  src/classes/Container.cpp \
  src/classes/Atom.cpp \
  src/classes/Particle.cpp \
  src/classes/SubAtomicParticle.cpp \
  src/physics/QmSurface1D.cpp \
  -o test_Simulation.exe

./test_Simulation.exe

*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <fstream> 
#include "../src/config.hpp"
#include "../src/classes/Simulation.hpp"
#include "../src/physics/QmSurface1D.hpp"

#include <glm/glm.hpp>

static void requireVecApprox(const glm::vec3& a, const glm::vec3& b, float eps = 1e-6f) {
    REQUIRE(a.x == Approx(b.x).margin(eps));
    REQUIRE(a.y == Approx(b.y).margin(eps));
    REQUIRE(a.z == Approx(b.z).margin(eps));
}

TEST_CASE("Simulation constructs with requested number of particles") {
    Config cfg;
    cfg.numParticles = 1;

    // Keep it simple / deterministic
    cfg.enable_chemistry = false;
    cfg.particleMass = 1.0f;
    cfg.particleRadius = 0.1f;
    cfg.spawnRandomParticles = true;

    // Set a valid container (avoid uninitialized walls in default Container())
    cfg.containerMinX = cfg.containerMinY = cfg.containerMinZ = -2.0f;
    cfg.containerMaxX = cfg.containerMaxY = cfg.containerMaxZ =  2.0f;

    Simulation sim(cfg);

    REQUIRE(sim.container.particles.size() == 1);
    REQUIRE(sim.container.particles[0].mass == Approx(1.0f));
    REQUIRE(sim.container.particles[0].radius == Approx(0.1f));
}

TEST_CASE("Simulation update moves particles by v*dt when chemistry is disabled") {
    Config cfg;
    cfg.numParticles = 1;
    cfg.enable_chemistry = false;

    cfg.particleMass = 1.0f;
    cfg.particleRadius = 0.1f;

    cfg.containerMinX = cfg.containerMinY = cfg.containerMinZ = -10.0f;
    cfg.containerMaxX = cfg.containerMaxY = cfg.containerMaxZ =  10.0f;

    Simulation sim(cfg);

    // Override random init so the test is deterministic
    sim.container.particles[0].position = glm::vec3(0.0f, 0.0f, 0.0f);
    sim.container.particles[0].velocity = glm::vec3(1.0f, 2.0f, 3.0f);

    float dt = 1.0f;
    sim.update(dt);

    requireVecApprox(sim.container.particles[0].position, glm::vec3(1.0f, 2.0f, 3.0f), 1e-5f);
}

TEST_CASE("Simulation::checkReactions forms bond for close H atoms with sufficient energy") {
    // Arrange: Config with barrier
    Config cfg;
    cfg.activation_barrier = 0.1f;  // Low for test
    cfg.spawnRandomParticles = false;  // Manual

    Simulation sim(cfg);
    // Add two H atoms close with high relative speed
    // KE ~0.5*1836*1^2 = high
    sim.atoms.emplace_back("H", 1, 1836.0f, 1.0f, 0.53f, 1.0f, 0.53f, 13.6f, 1.0f, glm::vec3(-0.6f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));  
    sim.atoms.emplace_back("H", 1, 1836.0f, 1.0f, 0.53f, 1.0f, 0.53f, 13.6f, 1.0f, glm::vec3(0.6f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f));

    float initial_dist = glm::length(sim.atoms[1].nucleus.position - sim.atoms[0].nucleus.position);
    glm::vec3 v0_before = sim.atoms[0].nucleus.velocity;
    glm::vec3 v1_before = sim.atoms[1].nucleus.velocity;

    // Act: Call checkReactions
    sim.checkReactions();

    // Assert: Positions adjusted to ~0.74 Å apart, centered
    float final_dist = glm::length(sim.atoms[1].nucleus.position - sim.atoms[0].nucleus.position);
    REQUIRE(final_dist == Approx(0.74f).margin(0.01f));
    REQUIRE(initial_dist > final_dist);  // Closer now

    // Velocities damped
    REQUIRE(glm::length(sim.atoms[0].nucleus.velocity) < glm::length(v0_before));
    REQUIRE(glm::length(sim.atoms[1].nucleus.velocity) < glm::length(v1_before));
    REQUIRE(sim.atoms[0].bonded_to.size() == 1); REQUIRE(sim.atoms[0].bond_strength > 0.0f);
}

TEST_CASE("Simulation loads element data from JSON correctly") {
    // Arrange: Create temp JSON file for test
    std::string json_filename = "test_elements.json";
    std::ofstream json_file(json_filename);
    json_file << R"({
      "H": {
        "Z": 1,
        "mass": 1836.0,
        "charge": 1.0,
        "vdw_r": 0.53,
        "harmonic_k": 1.0,
        "harmonic_eq": 0.53,
        "barrier": 0.1
      }
    })";
    json_file.close();

    Config cfg;
    Simulation sim(cfg);

    // Act: Load JSON
    sim.loadElementsFromJSON(json_filename);

    // Assert: Params loaded (assume sim has std::map<std::string, ElementData> elementData;)
    REQUIRE(sim.elementData.size() == 1);
    const auto& h_data = sim.elementData["H"];
    REQUIRE(h_data.Z == 1);
    REQUIRE(h_data.mass == Approx(1836.0f));
    REQUIRE(h_data.charge == Approx(1.0f));
    REQUIRE(h_data.vdw_r == Approx(0.53f));
    REQUIRE(h_data.harmonic_k == Approx(1.0f));
    REQUIRE(h_data.harmonic_eq == Approx(0.53f));
    REQUIRE(h_data.barrier == Approx(0.1f));

    // Cleanup
    std::remove(json_filename.c_str());
}

TEST_CASE("Simulation QM surface step moves H2 nuclei to lower/higher energy appropriately") {
    Config cfg;
    cfg.enable_subatomic = true;
    cfg.spawnRandomParticles = false;

    // isolate QM-only forces (no Atom::applyForces damping / coulomb / etc.)
    cfg.enable_qm_surface_h2 = true;
    cfg.enable_atom_forces = false;

    Simulation sim(cfg);

    // Fake H2 PES: min at 0.74
    QmSurface1D surf({
        {0.50f,  5.0f},
        {0.74f,  0.0f},
        {1.20f,  5.0f},
        {2.00f,  8.0f}
    });

    sim.setQmSurfaceH2(&surf);

    auto reset_two_H = [&](float R) {
        sim.atoms.clear();

        // Toy mass so 1 step produces measurable displacement
        const float m = 1.0f;

        sim.atoms.emplace_back("H", 1,
            /*mass*/ m, /*charge*/ 1.0f, /*vdw_r*/ 0.53f,
            /*h_k*/ 1.0f, /*h_eq*/ 0.53f,
            /*morse_De*/ 13.6f, /*morse_a*/ 1.0f,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f)
        );

        sim.atoms.emplace_back("H", 1,
            /*mass*/ m, /*charge*/ 1.0f, /*vdw_r*/ 0.53f,
            /*h_k*/ 1.0f, /*h_eq*/ 0.53f,
            /*morse_De*/ 13.6f, /*morse_a*/ 1.0f,
            glm::vec3(R, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f)
        );
    };

    auto current_R = [&]() -> float {
        return glm::length(sim.atoms[1].nucleus.position - sim.atoms[0].nucleus.position);
    };

    const float dt = 1e-3f;
    const float epsR = 1e-6f;

    SECTION("R < Re: nuclei move apart and energy decreases") {
        reset_two_H(0.55f);

        float R0 = current_R();
        float E0 = surf.E(R0);

        sim.update(dt);

        float R1 = current_R();
        float E1 = surf.E(R1);

        REQUIRE(R1 > R0 + epsR);  // moved apart (repulsion)
        REQUIRE(E1 < E0);         // moved toward Re => downhill in energy
    }
}

TEST_CASE("Simulation QM surface forces conserve total momentum (equal-and-opposite)") {
    Config cfg;
    cfg.enable_subatomic = true;
    cfg.spawnRandomParticles = false;
    cfg.enable_qm_surface_h2 = true;
    cfg.enable_atom_forces = false;
    cfg.enable_qm_bond_damping = true;
    cfg.enable_damping = true;
    Simulation sim(cfg);
    QmSurface1D surf({
        {0.50f, 5.0f},
        {0.74f, 0.0f},
        {1.20f, 5.0f},
        {2.00f, 8.0f}
    });
    sim.setQmSurfaceH2(&surf);
    sim.atoms.clear();
    const float m = 1.0f;
    sim.atoms.emplace_back("H", 1,
        m, 1.0f, 0.53f,
        1.0f, 0.53f,
        13.6f, 1.0f,
        glm::vec3(-0.55f, 0.0f, 0.0f),
        glm::vec3(0.00f, 0.0f, 0.0f)
    );
    sim.atoms.emplace_back("H", 1,
        m, 1.0f, 0.53f,
        1.0f, 0.53f,
        13.6f, 1.0f,
        glm::vec3(+0.55f, 0.0f, 0.0f),
        glm::vec3(0.00f, 0.0f, 0.0f)
    );
    auto momentum = [&]() -> glm::vec3 {
        return sim.atoms[0].nucleus.mass * sim.atoms[0].nucleus.velocity +
               sim.atoms[1].nucleus.mass * sim.atoms[1].nucleus.velocity;
    };
    const float dt = 1e-3f;
    glm::vec3 p0 = momentum();
    sim.update(dt);
    glm::vec3 p1 = momentum();
    REQUIRE(p1.x == Approx(p0.x).margin(1e-6f));
    REQUIRE(p1.y == Approx(p0.y).margin(1e-6f));
    REQUIRE(p1.z == Approx(p0.z).margin(1e-6f));
    REQUIRE(sim.atoms[0].nucleus.velocity.x == Approx(-sim.atoms[1].nucleus.velocity.x).margin(1e-6f));
}

TEST_CASE("QM H2 with bond damping converges near Re and settles") {
    Config cfg;
    cfg.enable_subatomic = true;
    cfg.spawnRandomParticles = false;
    // QM + damping (but still no Atom::applyForces)
    cfg.enable_qm_surface_h2 = true;
    cfg.enable_atom_forces = false;
    cfg.enable_qm_bond_damping = true;
    cfg.qm_bond_damping_gamma = 20.0f;
    cfg.enable_damping = false;
    Simulation sim(cfg);
    // Fake H2 PES: min at 0.74; energies in arbitrary units—scale if needed for realistic forces
    QmSurface1D surf({
        {0.50f, 5.0f},
        {0.74f, 0.0f},
        {1.20f, 5.0f},
        {2.00f, 8.0f}
    });
    sim.setQmSurfaceH2(&surf);
    auto reset_two_H = [&](float R) {
        sim.atoms.clear();
        // Use unit mass for test speed; in prod, use real proton mass (1836 a.u.) with 
        // adjusted dt/steps or scaled PES forces
        const float m = 1.0f;
        sim.atoms.emplace_back("H", 1,
            m, 1.0f, 0.53f,
            1.0f, 0.53f,
            13.6f, 1.0f,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f)
        );
        sim.atoms.emplace_back("H", 1,
            m, 1.0f, 0.53f,
            1.0f, 0.53f,
            13.6f, 1.0f,
            glm::vec3(R, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f)
        );
    };
    auto current_R = [&]() -> float {
        return glm::length(sim.atoms[1].nucleus.position - sim.atoms[0].nucleus.position);
    };
    auto radial_speed = [&]() -> float {
        glm::vec3 r = sim.atoms[1].nucleus.position - sim.atoms[0].nucleus.position;
        float R = glm::length(r);
        if (R < 1e-8f) return 0.0f;
        glm::vec3 u = r / R;
        glm::vec3 vrel = sim.atoms[1].nucleus.velocity - sim.atoms[0].nucleus.velocity;
        return glm::dot(vrel, u);
    };
    // Helper: Reduced mass for energy calcs
    auto reduced_mass = [&]() -> float {
        float m1 = sim.atoms[0].nucleus.mass;
        float m2 = sim.atoms[1].nucleus.mass;
        return m1 * m2 / (m1 + m2);
    };
    // Helper: Radial KE (check damping reduces it)
    auto radial_ke = [&]() -> float {
        float v_rad = radial_speed();
        return 0.5f * reduced_mass() * v_rad * v_rad;
    };
    const float dt = 1e-4f;
    const int steps = 30000; // small dt; many steps
    SECTION("Compressed bond relaxes to Re") {
        reset_two_H(0.55f);
        float ke_before = radial_ke();
        REQUIRE(ke_before == Approx(0.0f));
        for (int i = 0; i < steps; ++i) sim.update(dt);
        REQUIRE(current_R() == Approx(0.74f).margin(0.02f));
        REQUIRE(std::abs(radial_speed()) < 0.1f); // Forgiving tolerance
        REQUIRE(radial_ke() < 0.5f); // Damping reduces energy
    }
}

TEST_CASE("Dynamic QM PES: computes on-the-fly and drives H2 relaxation") {
    Config cfg;
    cfg.enable_subatomic = true;
    cfg.spawnRandomParticles = false;
    cfg.enable_qm_surface_h2 = true;  // Enables QM mode
    cfg.enable_dynamic_qm = true;     // New flag: use on-the-fly QM if no PES
    cfg.enable_atom_forces = false;
    cfg.enable_qm_bond_damping = true;
    cfg.enable_damping = false;
    cfg.qm_bond_damping_gamma = 20.0f;
    Simulation sim(cfg);
    // No precomputed PES set—sim should fall back to dynamic QM
    // sim.setQmSurfaceH2(nullptr); // Explicitly none
    auto reset_two_H = [&](float R) {
        sim.atoms.clear();
        const float m = 1.0f;  // Unit mass for test
        sim.atoms.emplace_back("H", 1, m, 1.0f, 0.53f, 1.0f, 0.53f, 13.6f, 1.0f,
                               glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        sim.atoms.emplace_back("H", 1, m, 1.0f, 0.53f, 1.0f, 0.53f, 13.6f, 1.0f,
                               glm::vec3(R, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    };
    auto current_R = [&]() -> float {
        return glm::length(sim.atoms[1].nucleus.position - sim.atoms[0].nucleus.position);
    };
    auto radial_speed = [&]() -> float {
        glm::vec3 r = sim.atoms[1].nucleus.position - sim.atoms[0].nucleus.position;
        float R = glm::length(r);
        if (R < 1e-8f) return 0.0f;
        glm::vec3 u = r / R;
        glm::vec3 vrel = sim.atoms[1].nucleus.velocity - sim.atoms[0].nucleus.velocity;
        return glm::dot(vrel, u);
    };
    const float dt = 1e-4f;
    const int steps = 30000;
    SECTION("Compressed: dynamic QM relaxes to approx Re") {
        reset_two_H(0.55f);
        for (int i = 0; i < steps; ++i) sim.update(dt);
        REQUIRE(current_R() == Approx(0.74f).margin(0.1f));
        REQUIRE(std::abs(radial_speed()) < 0.1f);
    }
}

TEST_CASE("Simulation computeChemicalForces applies LJ repulsion/attraction correctly") {
    Config cfg;
    cfg.spawnRandomParticles = false;
    cfg.numParticles = 0;
    cfg.enable_chemistry = true;  // Enables LJ
    cfg.lj_epsilon = 1.0f;
    cfg.lj_sigma = 1.0f;
    Simulation sim(cfg);
    sim.container.particles.clear();
    // Close pair (repulsion): dist=0.8 < sigma, should repel
    sim.container.particles.emplace_back(1.0f, 0.1f, glm::vec3(-0.4f, 0.0f, 0.0f), glm::vec3(0.0f));
    sim.container.particles.emplace_back(1.0f, 0.1f, glm::vec3(0.4f, 0.0f, 0.0f), glm::vec3(0.0f));
    glm::vec3 p_before = sim.container.particles[0].velocity + sim.container.particles[1].velocity;
    const float dt = 0.001f;
    sim.computeChemicalForces(dt);
    REQUIRE(sim.container.particles[0].velocity.x < 0.0f);  // Left moves more left
    REQUIRE(sim.container.particles[1].velocity.x > 0.0f);  // Right moves more right
    glm::vec3 p_after = sim.container.particles[0].velocity + sim.container.particles[1].velocity;
    requireVecApprox(p_after, p_before, 1e-6f);  // Momentum conserved
}

TEST_CASE("Simulation tryFormBonds creates one bond for approaching particles and no duplicates") {
    Config cfg;
    cfg.spawnRandomParticles = false;
    cfg.numParticles = 0;
    cfg.enable_bonds = true;
    cfg.bond_form_dist = 1.5f;
    cfg.bond_r0 = 0.74f;
    cfg.bond_De = 4.52f;
    cfg.bond_a = 1.0f;
    cfg.enable_chemistry = false;  // Isolate bonding
    cfg.enable_coulomb = false;
    cfg.containerMinX = cfg.containerMinY = cfg.containerMinZ = -10.0f;
    cfg.containerMaxX = cfg.containerMaxY = cfg.containerMaxZ = 10.0f;
    Simulation sim(cfg);
    sim.container.particles.clear();
    sim.container.particles.emplace_back(1.0f, 0.1f, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    sim.container.particles.emplace_back(1.0f, 0.1f, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
    const float dt = 0.001f;
    for (int step = 0; step < 500; ++step) {
        sim.update(dt);  // Will call tryFormBonds
    }
    REQUIRE(sim.bonds.size() == 1);
    REQUIRE(sim.bonds[0].strength == Approx(1.0f));  // Or check other props
    // Continue stepping: no more bonds
    for (int step = 0; step < 500; ++step) sim.update(dt);
    REQUIRE(sim.bonds.size() == 1);
}

TEST_CASE("Simulation loadReactionsFromJSON parses JSON correctly") {
    // Arrange: Temp JSON file
    std::string json_filename = "test_reactions.json";
    std::ofstream json_file(json_filename);
    json_file << R"({
        "reactions": [
            {
                "name": "H+H->H2",
                "reactants": ["H", "H"],
                "product": "H2",
                "min_dist": 1.5,
                "min_ke": 0.1,
                "bond_energy": 4.52
            }
        ]
    })";
    json_file.close();
    Config cfg;
    Simulation sim(cfg);
    // Act: Load
    sim.loadReactionsFromJSON(json_filename);
    // Assert: Loaded one reaction with correct params
    REQUIRE(sim.reactions.size() == 1);
    const auto& r = sim.reactions[0];
    REQUIRE(r.name == "H+H->H2");
    REQUIRE(r.reactants.size() == 2);
    REQUIRE(r.reactants[0] == "H");
    REQUIRE(r.reactants[1] == "H");
    REQUIRE(r.product == "H2");
    REQUIRE(r.min_dist == Approx(1.5f));
    REQUIRE(r.min_ke == Approx(0.1f));
    REQUIRE(r.bond_energy == Approx(4.52f));
    // Cleanup
    std::remove(json_filename.c_str());
}

TEST_CASE("Simulation checkReactions uses loaded JSON rule to form bond") {
    Config cfg;
    cfg.activation_barrier = 0.1f;  // Fallback, but test uses JSON
    cfg.spawnRandomParticles = false;
    Simulation sim(cfg);
    // Load custom JSON rule
    std::string json_filename = "test_reactions.json";
    std::ofstream json_file(json_filename);
    json_file << R"({
        "reactions": [
            {
                "name": "H+H->H2",
                "reactants": ["H", "H"],
                "product": "H2",
                "min_dist": 1.5,
                "min_ke": 0.1,
                "bond_energy": 4.52
            }
        ]
    })";
    json_file.close();
    sim.loadReactionsFromJSON(json_filename);
    std::remove(json_filename.c_str());
    // Add close H atoms with sufficient KE
    sim.atoms.emplace_back("H", 1, 1836.0f, 1.0f, 0.53f, 1.0f, 0.53f, 13.6f, 1.0f, glm::vec3(-0.6f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    sim.atoms.emplace_back("H", 1, 1836.0f, 1.0f, 0.53f, 1.0f, 0.53f, 13.6f, 1.0f, glm::vec3(0.6f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
    // Act
    sim.checkReactions();
    // Assert: Bond formed via rule (not fallback)
    REQUIRE(sim.atoms[0].bonded_to.size() == 1);
    REQUIRE(sim.atoms[0].bond_strength == Approx(4.52f));
    REQUIRE(glm::length(sim.atoms[1].nucleus.position - sim.atoms[0].nucleus.position) == Approx(0.74f).margin(0.01f));
    REQUIRE(glm::length(sim.atoms[0].nucleus.velocity) < glm::length(glm::vec3(1.0f, 0.0f, 0.0f)));  // Damped
}

TEST_CASE("Simulation applyForces binds electron to nucleus with harmonic potential") {
    Config cfg;
    cfg.enable_subatomic = true;
    cfg.spawnRandomParticles = false;
    cfg.enable_atom_forces = true;  // Enables applyForces
    cfg.enable_qm_surface_h2 = false;
    cfg.enable_coulomb = false;  // Isolate harmonic
    cfg.electron_mass = 1.0f;
    Simulation sim(cfg);
    sim.atoms.clear();
    sim.atoms.emplace_back("H", 1, 1836.0f, 1.0f, 0.53f, 1.0f, 0.53f, 13.6f, 1.0f, glm::vec3(0.0f), glm::vec3(0.0f));
    std::vector<double> e_pos = {1.0f, 0.0f, 0.0f};  // Displaced > eq=0.53
    std::vector<double> e_vel = {0.0f, 0.0f, 0.0f};
    sim.atoms[0].electrons.emplace_back(1.0f, e_pos, e_vel, -1.0f, 1);
    glm::vec3 p_before(0.0f);
    for (const auto& atom : sim.atoms) {
        p_before += atom.nucleus.mass * atom.nucleus.velocity;
        for (const auto& e : atom.electrons) {
            auto e_v = e.getVelocity();
            p_before += cfg.electron_mass * glm::vec3(e_v[0], e_v[1], e_v[2]);
        }
    }
    const float dt = 0.001f;
    sim.atoms[0].applyForces(sim.atoms, dt, cfg);
    // Assert: Electron accelerates toward nucleus
    REQUIRE(sim.atoms[0].electrons[0].getVelocity()[0] < 0.0f);
    // Momentum conserved
    glm::vec3 p_after(0.0f);
    for (const auto& atom : sim.atoms) {
        p_after += atom.nucleus.mass * atom.nucleus.velocity;
        for (const auto& e : atom.electrons) {
            auto e_v = e.getVelocity();
            p_after += cfg.electron_mass * glm::vec3(e_v[0], e_v[1], e_v[2]);
        }
    }
    requireVecApprox(p_after, p_before, 1e-5f);
}

TEST_CASE("Simulation update with dt=0 does nothing") {
    Config cfg;
    cfg.spawnRandomParticles = false;
    cfg.numParticles = 0;
    cfg.enable_subatomic = false;
    cfg.enable_chemistry = false;
    cfg.enable_coulomb = false;
    Simulation sim(cfg);
    sim.container.particles.emplace_back(1.0f, 0.1f, glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 pos_before = sim.container.particles[0].position;
    glm::vec3 vel_before = sim.container.particles[0].velocity;
    sim.update(0.0f);
    requireVecApprox(sim.container.particles[0].position, pos_before);
    requireVecApprox(sim.container.particles[0].velocity, vel_before);
}

TEST_CASE("Simulation update with >2 atoms skips QM gracefully (no crash)") {
    Config cfg;
    cfg.enable_subatomic = true;
    cfg.spawnRandomParticles = false;
    cfg.enable_qm_surface_h2 = true;
    cfg.enable_dynamic_qm = true;
    cfg.enable_atom_forces = false;
    Simulation sim(cfg);
    sim.atoms.clear();
    const float m = 1.0f;
    // Add 3 H atoms
    sim.atoms.emplace_back("H", 1, m, 1.0f, 0.53f, 1.0f, 0.53f, 13.6f, 1.0f, glm::vec3(0.0f), glm::vec3(0.0f));
    sim.atoms.emplace_back("H", 1, m, 1.0f, 0.53f, 1.0f, 0.53f, 13.6f, 1.0f, glm::vec3(1.0f), glm::vec3(0.0f));
    sim.atoms.emplace_back("H", 1, m, 1.0f, 0.53f, 1.0f, 0.53f, 13.6f, 1.0f, glm::vec3(2.0f), glm::vec3(0.0f));
    const float dt = 0.001f;
    // Act: Update should not crash (QM limited to 2 atoms)
    sim.update(dt);
    // Assert: No changes (or custom handling if implemented)
    REQUIRE(sim.atoms[0].nucleus.velocity == glm::vec3(0.0f));
}

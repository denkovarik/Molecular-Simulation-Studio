# Molecular Simulation Studio

**An interactive, visual molecular dynamics and chemistry simulator**  
From bouncing balls to real organic and biochemical reactions — built in C++ with OpenGL.

## Project Vision & Goals

### Short-Term
- Simulate fundamental organic chemistry reactions with clear visual electron flow  
  (e.g., H + H → H₂, CH₃Cl + OH⁻ → CH₃OH + Cl⁻, H₂ + ½O₂ → H₂O)
- Make reaction mechanisms intuitive for students who struggle with 2D arrow-pushing
- Deliver a beautiful, real-time 3D demo suitable for portfolio and web deployment on a website

### Medium-Term
- Accurately handle chemical “exceptions” (electronegativity, periodic trends, lone pairs, etc.)
- Support reactive simulations where bonds form and break dynamically
- Add simplified quantum-inspired effects (orbital shapes, partial charges, electron density)

### Long-Term
- Scale to large biochemical systems (e.g., ATP synthesis, oxidative phosphorylation, enzyme mechanisms)
- Remain fast enough for interactive or near-real-time use on consumer hardware or in browsers

## High-Level Technical Approach (Chosen Strategy)

We are following a **phased, scalable, hybrid strategy** that starts simple and evolves toward quantum accuracy over time:

| Phase | Core Method                        | Accuracy | Scalability | When |
|-------|------------------------------------|----------|-------------|------|
| 1     | Classical MD + Data-Driven Force Fields | Good for education & most organics | Excellent (10⁴–10⁶ atoms) | Now (starting point) |
| 2     | Reactive Force Fields (ReaxFF-style) + Empirical Rules | Handles bond breaking/forming | Very good | Next few months |
| 3     | QM/MM Hybrid (semi-empirical or DFT for reactive region) | Chemical accuracy where needed | Good (10³ atoms) | Mid-term |
| 4     | ML-accelerated potentials (ANI, SchNet, MACE, etc.) or Quantum Computer interface | Near-quantum accuracy + speed | Excellent | Long-term / future-proof |

**We begin with Phase 1** — this gives us immediate results, full control in C++, and a solid foundation to layer more accurate methods later.

### Phase 1 Architecture (Classical + Data-Driven)

- Atoms = `Particle` / `SubAtomicParticle` objects with:
  - Element type (H, C, N, O, …)
  - Mass, van-der-Waals radius, partial charge
  - Lennard-Jones parameters (ε, σ) from literature/QM databases
- Forces calculated every timestep:
  - Coulomb electrostatics between partial charges
  - Lennard-Jones 12-6 for Pauli repulsion + dispersion
  - Optional harmonic bonds/angles/dihedrals (non-reactive or switchable)
- Reactions triggered by simple, data-driven rules:
  - Distance + energy thresholds → bond formation/breaking
  - Activation energies and electronegativity differences from tables
- Electron “flow” visualized via partial charges, bond lines, or translucent density clouds

This approach is **fast, stable, extensible**, and already fits perfectly into the existing codebase**.

### Future Evolution Path

```text
Phase 1 → Phase 2 → Phase 3 → Phase 4
Classical + Data   Reactive FF   QM/MM   ML-potentials / Quantum hardware
    ↑                  ↑           ↑            ↑
   Fast & scalable    Bonds break Accurate reactions Near-exact + fast
```

We can replace or augment components **incrementally without rewriting the engine**.  
Every new layer — whether a better force field, a reactive bond-order model, a QM/MM subsystem, or a machine-learning potential — simply provides more accurate forces or reaction rules while reusing the same particle system, spatial grid, integrator, and OpenGL renderer.

### Current Repository Structure (Summary)
```text
src/
├── classes/
│   ├── Particle.hpp / Particle.cpp
│   ├── SubAtomicParticle.hpp / SubAtomicParticle.cpp
│   ├── Simulation.hpp / Simulation.cpp
│   ├── Container.hpp / Container.cpp        # spatial grid + collision handling
│   └── Renderer.hpp / Renderer.cpp          # OpenGL + sphere rendering
├── config.hpp
└── main.cpp
testing/                                         # Catch2 unit tests
```


The code is deliberately modular, well-tested, and ready for the phased upgrades described above.

## Why This Approach Wins

- Immediate visual results → perfect for portfolio & education
- Naturally handles chemical “exceptions” via data tables instead of rigid rules
- Scales to biochemical complexity today (unlike full QM)
- Clear, low-risk upgrade path to near-quantum accuracy tomorrow
- Stays 100% in C++ → no Python dependency hell, easy to compile to WebAssembly later

## Contributing & Next Steps

1. Extend `Particle` with element types and load parameters from JSON/CSV  
2. Implement Coulomb + Lennard-Jones forces  
3. Add simple reactive rules (start with H + H → H₂)  
4. Visualize bonds and partial charges  
5. Iterate → reactive organics → biochemistry  

Join us — let’s build the molecular simulator we all wish we had in class!

---
**License**: MIT  
**Languages**: C++17, GLSL, GLM  
**Build**: CMake (in progress)  
**Status**: Active development — Phase 1 underway

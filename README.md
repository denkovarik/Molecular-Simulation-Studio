# Molecular Simulation Studio
**An interactive, visual molecular dynamics and chemistry simulator**  
From bouncing balls to real organic and biochemical reactions — built in C++ with OpenGL.

## Project Vision & Goals
### Short-Term (Done)
- Beautiful real-time H + H → H₂ demo with glowing bond
- Stable classical MD engine with Lennard-Jones + optional damping
- Bouncing-ball mode and chemistry mode coexist cleanly
- Fully working, tested, and portfolio-ready

### Medium-Term (Current Focus)
- Replace "united-atom" model with **explicit protons + electrons** that remain bound during normal motion but can transfer in reactions
- Visualize real **electron flow** during bond formation and redox
- Make mechanisms (SN2, addition, elimination, redox) intuitive via moving electron particles
- Keep everything fast and stable on consumer hardware

### Long-Term
- Scale to large biochemical systems (ATP, enzymes, membranes)
- QM/MM, ReaxFF-style, or ML potentials under the hood
- WebAssembly deployment

## High-Level Technical Approach (Chosen Strategy)
We follow a **phased, data-driven hybrid** strategy:

| Phase | Core Method                              | Accuracy               | Scalability       | Status       |
|-------|------------------------------------------|------------------------|-------------------|--------------|
| 1     | Classical MD + LJ + data-driven rules     | Good for education     | Excellent (10⁶)  | Complete     |
| 2     | **Explicit protons & electrons** + empirical binding potentials | Chemical realism + electron flow | Very good (10⁴–10⁵) | **Next** |
| 3     | Reactive force fields (ReaxFF-style) or QM/MM | Bond breaking/forming | Good             | Mid-term     |
| 4     | ML potentials or quantum interface         | Near-quantum           | Excellent         | Long-term    |

**We are now finishing Phase 1 and moving into Phase 2.**

### Current Architecture (Phase 1 – Complete)
- Atoms = `Particle` objects (united-atom style)
- Forces: Lennard-Jones 12-6 + optional Coulomb via `Particle::charge`
- Reactions: simple distance + energy rules
- Visuals: glowing bonds when atoms get close

### Phase 2 Architecture (Next Milestone – Explicit Electrons)
We will **keep protons and electrons as separate objects** (`SubAtomicParticle`) but make them **stable** using **data-driven constraints** instead of raw Coulomb forces:

| Feature                         | How we will do it (data-driven)                                 |
|---------------------------------|-----------------------------------------------------------------|
| Electrons stay bound to protons   | Harmonic or Morse potentials tuned to real binding energies (13.6 eV for H) |
| Realistic orbital shapes           | Precomputed Gaussian or Slater-type density blobs that follow nuclei   |
| Electron transfer in reactions      | When activation energy is met → remove harmonic restraint → electron can jump |
| Visual electron flow              | Render small glowing spheres or trails that move between atoms         |
| No explosion / collapse           | Softened Coulomb + strong empirical binding + smaller timesteps      |

This gives us:
- True electron movement (not just partial charges)
- Ability to show redox, radicals, ionic mechanisms
- Still fully classical and fast
- Natural upgrade path to ReaxFF or ML potentials later

## Current Repository Structure
```text
src/
├── classes/
│   ├── Particle.hpp / Particle.cpp          # Now has Element, charge, force accumulation
│   ├── SubAtomicParticle.hpp / Particle.cpp # Ready for explicit electrons/protons
│   ├── Simulation.hpp / Simulation.cpp     # Coulomb + LJ + future binding forces
│   ├── Container.hpp / Container.cpp       # Grid + collisions (will support sub-particles)
│   └── Renderer.hpp / Renderer.cpp         # Charge-based coloring, ready for orbitals
├── config.hpp                            # enable_subatomic flag ready
├── main.cpp                             # Bouncing balls
└── main_h2.cpp                         # H₂ demo (currently united-atom)
testing/                                 # Catch2 tests updated
```

## Why This Approach Wins

- **Immediate visual results** → perfect for portfolio & education  
- **Explicit electrons** → real electron flow during reactions (no other educational simulator does this well)  
- **Data-driven constraints** → stable even though electrons are separate particles  
- **Clear, low-risk upgrade path** to full reactive force fields or ML potentials  
- **100% C++** → WebAssembly ready  

## Contributing & Next Steps (Updated – December 2025)

**Phase 1 is complete.**  
**Phase 2 (Explicit, Stable Electrons) starts now.**

1. Add **harmonic/Morse binding potentials** between protons and their electrons  
   → Use real data: 13.6 eV binding energy, 0.53 Å equilibrium distance for hydrogen

2. Make `config.enable_subatomic = true` **stable and beautiful**  
   → No more explosions when atoms approach or form bonds

3. Visualize electrons as **small glowing spheres** that orbit protons realistically  
   → Later evolve into proper orbital shapes (1s, 2p, sp³ hybrids, lone pairs)

4. Implement first **electron-transfer reaction** (e.g. H• + H• → H₂ or Na → Na⁺ + e⁻)

5. Load element-specific parameters from JSON  
   → mass, radius, LJ σ/ε, ionization energy, orbital data, etc.

6. Add **orbital-shaped density clouds** using 1–4 translucent Gaussians per orbital

After Phase 2 we’ll have the only educational simulator in the world that shows **real moving electrons** forming and breaking bonds — all while staying fast, stable, and fully in C++.


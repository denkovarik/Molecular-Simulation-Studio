# Makefile for Molecular Simulation Studio

CXX = g++
CXXFLAGS = -Wall -std=c++17 -O2
INCLUDES = -I src/classes -I src -I /usr/include
LIBS = -lglfw -lGL -lGLEW -lGLU -lX11 -lXxf86vm -lXrandr -lpthread -ldl -lXinerama -lXcursor

# ----------------------------
# Main application build
# ----------------------------
SOURCES = main.cpp \
          src/classes/Renderer.cpp \
          src/classes/Simulation.cpp \
          src/classes/Particle.cpp \
          src/classes/Container.cpp \
          src/classes/SubAtomicParticle.cpp \
          src/classes/Atom.cpp \
          src/physics/QmSurface1D.cpp

OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = MolDynSim

# ----------------------------
# H2 simple application build
# ----------------------------
H2_SOURCES = main_h2_simple.cpp \
             src/classes/Renderer.cpp \
             src/classes/Simulation.cpp \
             src/classes/Particle.cpp \
             src/classes/Container.cpp \
             src/classes/SubAtomicParticle.cpp \
             src/classes/Atom.cpp \
             src/physics/QmSurface1D.cpp

H2_OBJECTS = $(H2_SOURCES:.cpp=.o)
H2_EXECUTABLE = h2_simple

# ----------------------------
# Build all
# ----------------------------
all: $(EXECUTABLE) $(H2_EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LIBS) -o $@

$(H2_EXECUTABLE): $(H2_OBJECTS)
	$(CXX) $(H2_OBJECTS) $(LIBS) -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# ----------------------------
# Test build (Catch2 v2, header-only)
# ----------------------------
TEST_CXXFLAGS = -Wall -std=c++17 -O0 -g
TEST_INCLUDES = -I/usr/local/include -I src -I src/classes

TEST_BINS = test_Container.exe test_Particle.exe test_SubAtomicParticle.exe test_Simulation.exe test_CoulombForce.exe test_Atom.exe test_QmSurfaceH2.exe

tests: $(TEST_BINS)

test: tests
	@set -e; \
	echo "Running tests..."; \
	for t in $(TEST_BINS); do \
		echo "==> ./$$t"; \
		./$$t; \
	done; \
	echo "All tests passed."

test_Container.exe: testing/test_Container.cpp src/classes/Particle.cpp src/classes/Container.cpp
	$(CXX) $(TEST_CXXFLAGS) $(TEST_INCLUDES) -o $@ $^

test_Particle.exe: testing/test_Particle.cpp src/classes/Particle.cpp
	$(CXX) $(TEST_CXXFLAGS) $(TEST_INCLUDES) -o $@ $^

test_SubAtomicParticle.exe: testing/test_SubAtomicParticle.cpp src/classes/SubAtomicParticle.cpp
	$(CXX) $(TEST_CXXFLAGS) $(TEST_INCLUDES) -o $@ $^

test_Simulation.exe: testing/test_Simulation.cpp src/classes/Simulation.cpp src/classes/Container.cpp src/classes/Atom.cpp src/classes/Particle.cpp src/classes/SubAtomicParticle.cpp src/physics/QmSurface1D.cpp
	$(CXX) $(TEST_CXXFLAGS) $(TEST_INCLUDES) -o $@ $^
	
test_CoulombForce.exe: testing/test_CoulombForce.cpp src/classes/Simulation.cpp src/classes/Container.cpp src/classes/Particle.cpp src/classes/Atom.cpp src/classes/SubAtomicParticle.cpp src/physics/QmSurface1D.cpp
	$(CXX) $(TEST_CXXFLAGS) $(TEST_INCLUDES) -o $@ $^
	
test_Atom.exe: testing/test_Atom.cpp src/classes/Atom.cpp src/classes/Particle.cpp src/classes/Atom.cpp src/classes/SubAtomicParticle.cpp
	$(CXX) $(TEST_CXXFLAGS) $(TEST_INCLUDES) -o $@ $^
	
test_QmSurfaceH2.exe: testing/test_QmSurfaceH2.cpp src/physics/QmSurface1D.cpp
	$(CXX) $(TEST_CXXFLAGS) $(TEST_INCLUDES) -o $@ $^


# Convenience targets to run one suite
test_container: test_Container.exe
	./test_Container.exe

test_particle: test_Particle.exe
	./test_Particle.exe

test_subatomic: test_SubAtomicParticle.exe
	./test_SubAtomicParticle.exe

test_simulation: test_Simulation.exe
	./test_Simulation.exe
	
test_coulomb: test_CoulombForce.exe
	./test_CoulombForce.exe
	
test_atom: test_Atom.exe
	./test_Atom.exe

test_QmSurfaceH2: test_QmSurfaceH2.exe
	./test_QmSurfaceH2.exe
	
clean-tests:
	rm -f $(TEST_BINS)

clean:
	rm -f $(OBJECTS) $(H2_OBJECTS) $(EXECUTABLE) $(H2_EXECUTABLE) $(TEST_BINS)

.PHONY: all clean clean-tests tests test test_container test_particle test_subatomic test_simulation test_coulomb test_atom test_QmSurfaceH2


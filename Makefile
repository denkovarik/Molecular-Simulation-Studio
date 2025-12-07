# Makefile for Molecular Dynamics Simulator (Bouncing Balls evolving to Atoms/Molecules)

CXX = g++
CXXFLAGS = -Wall -std=c++17 -O2
INCLUDES = -I src/classes -I src -I /usr/include
LIBS = -lglfw -lGL -lGLEW -lGLU -lX11 -lXxf86vm -lXrandr -lpthread -ldl -lXinerama -lXcursor

# Source files (including SubAtomicParticle for future atomic/molecular simulations)
SOURCES = main.cpp \
          src/classes/Renderer.cpp \
          src/classes/Simulation.cpp \
          src/classes/Particle.cpp \
          src/classes/Container.cpp \
          src/classes/SubAtomicParticle.cpp

OBJECTS = $(SOURCES:.cpp=.o)

EXECUTABLE = MolDynSim

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LIBS) -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

#!/bin/bash

echo "Updating package list..."
apt update
apt upgrade -y

echo "Installing build-essentials"
apt install -y build-essential

apt-get install libgl1-mesa-dev libglfw3 libglfw3-dev

apt install libglm-dev

apt install libxxf86vm-dev libxinerama-dev libxcursor-dev

apt install libglew-dev

# OpenGL
apt-get install freeglut3-dev

# Install Catch2 (v2.13.10) into /usr/local/include (matches Makefile)
apt install -y git cmake

cd /tmp
git clone --depth 1 --branch v2.13.10 https://github.com/catchorg/Catch2.git
cd Catch2
cmake -B build -S . -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build


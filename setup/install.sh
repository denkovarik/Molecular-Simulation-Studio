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

# Install Catch2
cd ~
git clone -b v2.x https://github.com/catchorg/Catch2.git
cd Catch2
mkdir build && cd build
cmake .. -DBUILD_TESTING=OFF
make install

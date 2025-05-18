#!/bin/bash

echo "Updating package list..."
sudo apt update
sudo apt upgrade -y

echo "Installing build-essentials"
sudo apt install -y build-essential

sudo apt-get install libgl1-mesa-dev libglfw3 libglfw3-dev

sudo apt install libglm-dev

sudo apt install libxxf86vm-dev libxinerama-dev libxcursor-dev

# OpenGL
sudo apt-get install freeglut3-dev

# Install Catch2
cd ~
git clone -b v2.x https://github.com/catchorg/Catch2.git
cd Catch2
mkdir build && cd build
cmake .. -DBUILD_TESTING=OFF
sudo make install
#!/bin/bash

echo "Updating package list..."
sudo apt update
sudo apt upgrade -y

echo "Installing build-essentials"
sudo apt install -y build-essential

# Install Catch2
cd ~
git clone -b v2.x https://github.com/catchorg/Catch2.git
cd Catch2
mkdir build && cd build
cmake .. -DBUILD_TESTING=OFF
sudo make install
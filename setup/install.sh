#!/bin/bash

echo "Updating package list..."
sudo apt update

echo "Installing build-essentials"
sudo apt install -y build-essential

#!/bin/bash

install_cmake()
{
    if command -v cmake &> /dev/null; then
        echo "CMake is already installed."
        return
    fi

    echo "CMake not found. Attempting to install..."

    if command -v apt-get &> /dev/null; then
      sudo apt-get install cmake
    elif command -v pacman &> /dev/null; then
      sudo pacman -S --noconfirm cmake
    else
        echo "Error: Unsupported package manager. Please install CMake manually."
        exit 1
    fi
}

install_cmake

mkdir -p build
cd build/
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target VoxelGame
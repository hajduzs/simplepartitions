#!/bin/bash

mkdir build
mkdir build/cgallib
cd build/cgallib

# Install used libraries (for Debian-based distros)
apt-get update
apt-get -y install libcgal-dev cmake make curl 

curl -O https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
cp ../../src/dangerzone.cpp ./dangerzone.cpp
cp ../../src/CMakeLists.txt ./CMakeLists.txt

cmake .
make

# cp libpartition.so ../../libs/libpartition.so
cp dangerzone ../partitioner.x
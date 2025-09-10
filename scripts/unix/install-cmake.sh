#!/bin/bash
set -e

version=4.0.0

wget https://github.com/Kitware/CMake/releases/download/v$version/cmake-$version-linux-x86_64.tar.gz
tar -xzf cmake-$version-linux-x86_64.tar.gz
sudo cp -r cmake-$version-linux-x86_64/bin/* /usr/local/bin/
sudo cp -r cmake-$version-linux-x86_64/share/* /usr/local/share/
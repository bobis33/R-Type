#!/bin/bash
# Only work for LINUX Ubuntu system

function installVulkanSDK() {
    wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo tee /etc/apt/trusted.gpg.d/lunarg.asc
    sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-jammy.list http://packages.lunarg.com/vulkan/lunarg-vulkan-jammy.list
    sudo apt update -y && sudo apt upgrade -y
    sudo apt install vulkan-sdk -y
}

function installDoxygen() {
    wget https://www.doxygen.nl/files/doxygen-1.11.0.linux.bin.tar.gz && tar xf doxygen-1.11.0.linux.bin.tar.gz
    tar xf doxygen-1.11.0.linux.bin.tar.gz
    sudo cp doxygen-1.11.0/bin/doxygen /usr/local/bin/
}

sudo apt update -y

case $1 in
    build)
        sudo apt install -y libxkbcommon-dev xorg-dev libwayland-dev libudev-dev
        ;;
    doc)
        sudo apt install -y libgl1-mesa-dev qt6-base-dev texlive-latex-base texlive-latex-recommended texlive-latex-extra graphviz texlive-xetex
        installDoxygen
        ;;
    *)
        echo "Usage $0 build | doc"
        exit 1
        ;;
esac

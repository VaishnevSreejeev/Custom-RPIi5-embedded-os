#!/bin/bash
set -e

echo "Setting up Yocto layers for Raspberry Pi 5..."

# Create sources directory
mkdir -p sources

# Clone Poky (Scarthgap)
if [ ! -d "sources/poky" ]; then
    echo "Cloning Poky..."
    git clone -b scarthgap git://git.yoctoproject.org/poky sources/poky
else
    echo "Poky already exists."
fi

# Clone OpenEmbedded (Scarthgap)
if [ ! -d "sources/meta-openembedded" ]; then
    echo "Cloning Meta-OpenEmbedded..."
    git clone -b scarthgap https://github.com/openembedded/meta-openembedded.git sources/meta-openembedded
else
    echo "Meta-OpenEmbedded already exists."
fi

# Clone Meta-RaspberryPi (Scarthgap)
if [ ! -d "sources/meta-raspberrypi" ]; then
    echo "Cloning Meta-RaspberryPi..."
    git clone -b scarthgap https://github.com/agherzan/meta-raspberrypi.git sources/meta-raspberrypi
else
    echo "Meta-RaspberryPi already exists."
fi

echo "Layer setup complete."

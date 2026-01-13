# Embedded Linux for Raspberry Pi 5

A custom embedded Linux distribution built from scratch using the Yocto Project (Poky Scarthgap).

## Project Overview
This project demonstrates the creation of a minimal, production-ready Linux image for the Raspberry Pi 5. It leverages the OpenEmbedded build system to generate a custom OS tailerd for ARM64 architecture.

### Key Features
*   **Architecture**: aarch64 (ARM 64-bit)
*   **Base System**: Poky (Scarthgap Release)
*   **Hardware Layer**: Meta-RaspberryPi
*   **Init System**: SysVinit (Lightweight)
*   **Connectivity**: Full networking support including WiFi firmware configuration (`synaptics-killswitch` license handling).

## File Structure
*   `conf/`: Custom Yocto configuration (`local.conf`, `bblayers.conf`).
*   `full-setup.sh`: One-click script to fetch dependencies and initialize the workspace.

## How to Build

### 1. Prerequisites
Ubuntu/Debian host:
```bash
sudo apt install gawk wget git diffstat unzip texinfo gcc build-essential chrpath socat cpio python3 python3-pip python3-pexpect xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1 libsdl1.2-dev
```

### 2. Fetch Sources
Run the setup script to clone the required Yocto layers:
```bash
chmod +x full-setup.sh
./full-setup.sh
```

### 3. Initialize & Build
```bash
source sources/poky/oe-init-build-env build
bitbake core-image-base
```

### 4. Deploy
Flash the generated image to an SD card:
```bash
bzcat tmp/deploy/images/raspberrypi5/core-image-base-raspberrypi5.wic.bz2 | sudo dd of=/dev/sdX bs=4M status=progress
```

## Author
[Your Name]
Intern, Embedded Systems Engineer

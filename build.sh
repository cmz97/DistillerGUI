#!/bin/bash

# Exit on any error
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo -e "${YELLOW}Please run as root (sudo)${NC}"
    exec sudo "$0" "$@"
fi

# Install dependencies if needed
if [ "$(id -u)" = "0" ]; then
    echo -e "${YELLOW}Installing dependencies...${NC}"
    apt-get update
    apt-get install -y build-essential cmake git libgpiod-dev gpiod
else
    echo -e "${YELLOW}Note: Run with sudo to automatically install dependencies${NC}"
fi

echo -e "${YELLOW}Building DistillerGUI...${NC}"

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}Error: CMakeLists.txt not found. Are you in the right directory?${NC}"
    echo "Please run this script from the DistillerGUI root directory"
    exit 1
fi

# Check for required tools
echo -e "${YELLOW}Checking for required tools...${NC}"
command -v cmake >/dev/null 2>&1 || { echo -e "${RED}Error: cmake is required but not installed.${NC}" >&2; exit 1; }
command -v make >/dev/null 2>&1 || { echo -e "${RED}Error: make is required but not installed.${NC}" >&2; exit 1; }

# Check for required files
echo -e "${YELLOW}Checking for required files...${NC}"
required_files=("main.c" "eink_driver.c" "eink_driver.h" "lv_conf.h")
for file in "${required_files[@]}"; do
    if [ ! -f "$file" ]; then
        echo -e "${RED}Error: Required file $file not found${NC}"
        exit 1
    fi
done

# Clean build directory if it exists
echo -e "${YELLOW}Cleaning build directory...${NC}"
rm -rf build

# Create new build directory
echo -e "${YELLOW}Creating build directory...${NC}"
mkdir -p build
cd build

# Configure and build
echo -e "${YELLOW}Configuring with CMake...${NC}"
if cmake ..; then
    echo -e "${GREEN}CMake configuration successful${NC}"
else
    echo -e "${RED}CMake configuration failed${NC}"
    exit 1
fi

echo -e "${YELLOW}Building...${NC}"
if make -j$(nproc); then
    echo -e "${GREEN}Build successful!${NC}"
else
    echo -e "${RED}Build failed${NC}"
    exit 1
fi

# Check if the binary was created
if [ -f "DistillerGUI" ]; then
    echo -e "${GREEN}Build completed successfully. Binary created at: build/DistillerGUI${NC}"
else
    echo -e "${RED}Binary not found after build${NC}"
    exit 1
fi

cd .. 
#!/bin/bash
# bitN_setup.sh - bit(n) Setup Script for RP2040 & RP2350 Support
# This script configures the build environment and installs dependencies

set -e

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}bit(n) Emulator - Setup Script${NC}"
echo -e "${GREEN}Supporting: RP2040 & RP2350${NC}"
echo -e "${GREEN}========================================${NC}"

# Check for required tools
check_command() {
    if ! command -v "$1" &> /dev/null; then
        echo -e "${RED}Error: $1 is not installed${NC}"
        return 1
    fi
    echo -e "${GREEN}✓ Found $1: $(which $1)${NC}"
    return 0
}

echo -e "\n${YELLOW}Checking for required tools...${NC}"
check_command "cmake" || exit 1
check_command "gcc" || exit 1
check_command "make" || exit 1
check_command "python3" || exit 1

# Detect OS
OS="$(uname -s)"
ARCH="$(uname -m)"
echo -e "\n${YELLOW}System Information:${NC}"
echo "  OS: $OS"
echo "  Architecture: $ARCH"

# Install Python dependencies
echo -e "\n${YELLOW}Installing Python dependencies...${NC}"
python3 -m pip install --quiet pyelftools --break-system-packages || {
    echo -e "${RED}Failed to install pyelftools${NC}"
    exit 1
}
echo -e "${GREEN}✓ Python dependencies installed${NC}"

# Create build directory
echo -e "\n${YELLOW}Creating build directories...${NC}"
mkdir -p build
mkdir -p build/debug
mkdir -p build/release
mkdir -p bin
echo -e "${GREEN}✓ Build directories created${NC}"

# Detect C standard library location (for linking)
echo -e "\n${YELLOW}Detecting toolchain paths...${NC}"
GCC_PREFIX=$(gcc -print-file-name=include)
GCC_PREFIX="${GCC_PREFIX%/include}"
echo -e "${GREEN}✓ GCC prefix: $GCC_PREFIX${NC}"

# Create configuration file
echo -e "\n${YELLOW}Creating CMake configuration...${NC}"
cat > build/CMakeUserToolchain.cmake << 'EOF'
# bit(n) CMake Toolchain Configuration

# C Standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS_INIT "-Wall -Wextra -pedantic -Wshadow")

# Debug and Release flags
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG=1")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG=1")

# Architecture-specific flags
if(BITWISE_TARGET STREQUAL "RP2040")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv6-m -mthumb -mcpu=cortex-m0plus")
    add_definitions(-DRP2040_TARGET)
    add_definitions(-DNUM_CORES=2)
    add_definitions(-DSRAM_SIZE=0x42800)      # 264KB
    add_definitions(-DGPIO_PINS=30)
    add_definitions(-DPIO_BLOCKS=2)
elseif(BITWISE_TARGET STREQUAL "RP2350_ARM")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-m.main -mthumb -mcpu=cortex-m33")
    add_definitions(-DRP2350_TARGET)
    add_definitions(-DRP2350_ARM_MODE)
    add_definitions(-DNUM_CORES=2)
    add_definitions(-DSRAM_SIZE=0x84000)      # 520KB
    add_definitions(-DGPIO_PINS=48)
    add_definitions(-DPIO_BLOCKS=3)
elseif(BITWISE_TARGET STREQUAL "RP2350_RISCV")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=rv32imafc -mcmodel=medany")
    add_definitions(-DRP2350_TARGET)
    add_definitions(-DRP2350_RISCV_MODE)
    add_definitions(-DNUM_CORES=2)
    add_definitions(-DSRAM_SIZE=0x84000)      # 520KB
    add_definitions(-DGPIO_PINS=48)
    add_definitions(-DPIO_BLOCKS=3)
endif()

message(STATUS "Target: ${BITWISE_TARGET}")
message(STATUS "C Flags: ${CMAKE_C_FLAGS}")
EOF

echo -e "${GREEN}✓ CMake configuration created${NC}"

# Installation paths
echo -e "\n${YELLOW}Setup Summary:${NC}"
echo "  Build directory: $(pwd)/build"
echo "  Binary output: $(pwd)/bin"
echo "  Test directory: $(pwd)/tests"

echo -e "\n${GREEN}========================================${NC}"
echo -e "${GREEN}Setup Complete!${NC}"
echo -e "${GREEN}========================================${NC}"

echo -e "\n${YELLOW}Next Steps:${NC}"
echo "  1. Configure build for RP2040:"
echo "     cd build && cmake -DBITWISE_TARGET=RP2040 .."
echo ""
echo "  2. Or configure for RP2350 ARM:"
echo "     cd build && cmake -DBITWISE_TARGET=RP2350_ARM .."
echo ""
echo "  3. Or configure for RP2350 RISC-V:"
echo "     cd build && cmake -DBITWISE_TARGET=RP2350_RISCV .."
echo ""
echo "  4. Build the project:"
echo "     make -j\$(nproc)"
echo ""
echo "  5. Run tests:"
echo "     make test"

exit 0

#!/bin/bash

# bit(N) - Bitfield Language Compiler Setup Script
# Converts Nim-style syntax to machine code

echo "=== bit(N) Language - Setup Script ==="
echo

# Check dependencies
echo "[*] Checking dependencies..."

if ! command -v cmake &> /dev/null; then
    echo "[✗] cmake not found. Please install CMake."
    exit 1
fi
echo "[✓] cmake found: $(cmake --version | head -n1)"

if ! command -v gcc &> /dev/null; then
    echo "[✗] gcc not found. Please install GCC."
    exit 1
fi
echo "[✓] gcc found: $(gcc --version | head -n1)"

echo

# Create build directory
echo "[*] Creating build directory..."
mkdir -p build
cd build || exit 1

# Run CMake
echo "[*] Running CMake..."
cmake .. || exit 1
echo "[✓] CMake configuration complete"

echo

# Build
echo "[*] Building bit(N) compiler..."
make || exit 1

echo "[✓] Build complete!"
echo

# Copy binary to parent directory
cp bitN .. || true

echo "=== Setup Complete ==="
echo
echo "Usage: ./bitn <filename.bitn>"
echo "  Compiles bit(N) Nim-syntax files"
echo
echo "Options:"
echo "  -c 'code' - Compile inline code"
echo
echo "Example:"
echo "  ./bitn -c 'proc main(): u32 = return 0xFF'"
echo
echo "Documentation:"
echo "  - README.md - Language guide"
echo "  - IMPLEMENTATION.md - Architecture details"
echo "  - examples/ - Example programs"

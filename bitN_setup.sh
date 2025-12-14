#!/bin/bash
# bitN-setup.sh - Quick setup and test script

set -e

echo "=== bit(N) Language - Setup Script ==="
echo

# Check for required tools
echo "[*] Checking dependencies..."
if ! command -v cmake &> /dev/null; then
    echo "ERROR: cmake not found. Install with: sudo apt-get install cmake"
    exit 1
fi

if ! command -v gcc &> /dev/null; then
    echo "ERROR: gcc not found. Install with: sudo apt-get install build-essential"
    exit 1
fi

echo "[✓] cmake found: $(cmake --version | head -1)"
echo "[✓] gcc found: $(gcc --version | head -1)"
echo

# Create build directory
echo "[*] Creating build directory..."
mkdir -p build
cd build

# Configure with CMake
echo "[*] Running CMake..."
cmake ..
echo "[✓] CMake configuration complete"
echo

# Build the project
echo "[*] Building bit(N) compiler..."
make -j$(nproc)
echo "[✓] Build complete!"
echo

# Run basic tests
echo "[*] Running basic tests..."
echo

echo "--- Test 1: Simple function ---"
./bitN -c "fn main() -> u32 { return 42; }"
echo

echo "--- Test 2: Bit operations ---"
./bitN -c "fn test() -> u32 { return 0xFF00 & 0x00FF; }"
echo

echo "--- Test 3: Bit slice ---"
./bitN -c "fn extract(x: u32) -> u8 { return x[8:16]; }"
echo

echo "--- Test 4: Example file ---"
./bitN ../examples/extract_bits.bitn
echo

echo "[✓] All tests passed!"
echo
echo "=== Setup Complete ==="
echo
echo "Usage:"
echo "  ./build/bitN <source_file>      - Compile a .bitn file"
echo "  ./build/bitN -c '<code>'        - Compile inline code"
echo
echo "Example:"
echo "  ./build/bitN -c 'fn main() -> u32 { return 0xFF; }'"
echo
echo "Documentation:"
echo "  - README.md - Language guide"
echo "  - IMPLEMENTATION.md - Architecture details"
echo "  - examples/ - Example programs"

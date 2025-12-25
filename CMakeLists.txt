cmake_minimum_required(VERSION 3.10)
project(bitN C)

# C Standard Configuration
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ============================================================================
# COMPILER FLAGS
# ============================================================================

# Base compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter")

# Embedded-specific flags for firmware optimization
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdata-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")

# Security hardening
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FORTIFY_SOURCE=2")

# ============================================================================
# INCLUDE DIRECTORIES
# ============================================================================

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/backend/codegen
    ${CMAKE_SOURCE_DIR}/backend/linker
    ${CMAKE_SOURCE_DIR}/backend/build
)

# ============================================================================
# SOURCE FILES
# ============================================================================

set(SOURCES
    # Compiler frontend
    src/main.c
    src/token.c
    src/lexer.c
    src/ast.c
    src/parser.c
    src/type_system.c
    src/type_inference.c
    src/symbol_table.c

    # Backend modules
    backend/codegen/codegen.c
    backend/linker/linker_gen.c
)

# ============================================================================
# EXECUTABLE TARGET
# ============================================================================

add_executable(bitN ${SOURCES})

# ============================================================================
# LINKER FLAGS
# ============================================================================

target_link_options(bitN PRIVATE
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    -Wl,--as-needed
)

# ============================================================================
# BUILD TYPE CONFIGURATION
# ============================================================================

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS_DEBUG   "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-O2")

# ============================================================================
# VERSION INFORMATION
# ============================================================================

set(BITN_VERSION_MAJOR 0)
set(BITN_VERSION_MINOR 1)
set(BITN_VERSION_PATCH 0)
add_definitions(-DBITN_VERSION="${BITN_VERSION_MAJOR}.${BITN_VERSION_MINOR}.${BITN_VERSION_PATCH}")

# ============================================================================
# INSTALLATION
# ============================================================================

install(TARGETS bitN DESTINATION bin)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/backend/codegen/
        DESTINATION include/bitn/codegen
        FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ${CMAKE_SOURCE_DIR}/backend/linker/
        DESTINATION include/bitn/linker
        FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ${CMAKE_SOURCE_DIR}/backend/build/
        DESTINATION include/bitn/build
        FILES_MATCHING PATTERN "*.h")

# ============================================================================
# TESTING
# ============================================================================

enable_testing()

add_test(
    NAME parser_test
    COMMAND bitN -c "fn main() -> u32 { return 42; }"
)

add_test(
    NAME codegen_test
    COMMAND bitN --backend-test codegen
)

add_test(
    NAME linker_test
    COMMAND bitN --backend-test linker
)

# ============================================================================
# BUILD STATUS
# ============================================================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "bit(N) Compiler Configuration")
message(STATUS "========================================")
message(STATUS "Build Type:         ${CMAKE_BUILD_TYPE}")
message(STATUS "C Standard:         ${CMAKE_C_STANDARD}")
message(STATUS "Compiler:           ${CMAKE_C_COMPILER}")
message(STATUS "Installation Path:  ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Features Enabled:")
message(STATUS "  ✓ Code Generation Backend")
message(STATUS "  ✓ Linker Script Generation")
message(STATUS "  ✓ Build System Integration")
message(STATUS "  ✓ Embedded Optimizations")
message(STATUS "  ✓ Section Garbage Collection")
message(STATUS "  ✓ Memory Usage Reporting")
message(STATUS "  ✓ Testing Framework (3 tests)")
message(STATUS "========================================")
message(STATUS "")

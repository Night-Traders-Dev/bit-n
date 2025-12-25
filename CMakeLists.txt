cmake_minimum_required(VERSION 3.16)

project(bitN 
    VERSION 1.0.0
    DESCRIPTION "bit(N) - ARM & RISC-V Microcontroller Emulator"
    LANGUAGES C
)

# Default target
if(NOT DEFINED BITWISE_TARGET)
    set(BITWISE_TARGET "RP2040" CACHE STRING "Target microcontroller: RP2040, RP2350_ARM, RP2350_RISCV")
endif()

message(STATUS "Configuring for target: ${BITWISE_TARGET}")

# Global settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Source files - Core system
set(CORE_SOURCES
    src/core/registers.c
    src/core/execution.c
    src/core/exceptions.c
)

# Source files - ISA Decoders
set(ISA_ARM_SOURCES
    src/isa/arm/thumb2.c
    src/isa/arm/decoder.c
    src/isa/arm/executor.c
)

set(ISA_RISCV_SOURCES
    src/isa/riscv/decoder.c
    src/isa/riscv/executor.c
    src/isa/riscv/csr.c
)

# Source files - Memory
set(MEMORY_SOURCES
    src/memory/sram.c
    src/memory/flash_xip.c
    src/memory/mmu.c
)

# Source files - Bus
set(BUS_SOURCES
    src/bus/ahb_lite.c
    src/bus/apb.c
    src/bus/interconnect.c
)

# Source files - Peripherals
set(PERIPH_SOURCES
    src/periph/gpio.c
    src/periph/uart.c
    src/periph/spi.c
    src/periph/i2c.c
    src/periph/pwm.c
    src/periph/pio.c
    src/periph/dma.c
    src/periph/adc.c
    src/periph/timer.c
    src/periph/clock.c
    src/periph/watchdog.c
    src/periph/sio.c
)

# Source files - Target-specific
set(RP2040_SOURCES
    src/rp2040/rp2040.c
    src/rp2040/bootrom.c
)

set(RP2350_SOURCES
    src/rp2350/rp2350.c
    src/rp2350/bootrom.c
    src/rp2350/security.c
)

# Combine sources based on target
if(BITWISE_TARGET MATCHES "RP2040")
    set(TARGET_SOURCES ${RP2040_SOURCES})
    set(ALL_SOURCES
        ${CORE_SOURCES}
        ${ISA_ARM_SOURCES}
        ${MEMORY_SOURCES}
        ${BUS_SOURCES}
        ${PERIPH_SOURCES}
        ${TARGET_SOURCES}
    )
    add_definitions(-DRP2040_TARGET)
    add_definitions(-DNUM_CORES=2)
    add_definitions(-DSRAM_SIZE=0x42800)      # 264KB
    add_definitions(-DGPIO_PINS=30)
    add_definitions(-DPIO_BLOCKS=2)
    
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv6-m -mthumb -mcpu=cortex-m0plus")
    
elseif(BITWISE_TARGET MATCHES "RP2350_ARM")
    set(TARGET_SOURCES ${RP2350_SOURCES})
    set(ALL_SOURCES
        ${CORE_SOURCES}
        ${ISA_ARM_SOURCES}
        ${MEMORY_SOURCES}
        ${BUS_SOURCES}
        ${PERIPH_SOURCES}
        ${TARGET_SOURCES}
    )
    add_definitions(-DRP2350_TARGET)
    add_definitions(-DRP2350_ARM_MODE)
    add_definitions(-DNUM_CORES=2)
    add_definitions(-DSRAM_SIZE=0x84000)      # 520KB
    add_definitions(-DGPIO_PINS=48)
    add_definitions(-DPIO_BLOCKS=3)
    
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-m.main -mthumb -mcpu=cortex-m33")
    
elseif(BITWISE_TARGET MATCHES "RP2350_RISCV")
    set(TARGET_SOURCES ${RP2350_SOURCES})
    set(ALL_SOURCES
        ${CORE_SOURCES}
        ${ISA_RISCV_SOURCES}
        ${MEMORY_SOURCES}
        ${BUS_SOURCES}
        ${PERIPH_SOURCES}
        ${TARGET_SOURCES}
    )
    add_definitions(-DRP2350_TARGET)
    add_definitions(-DRP2350_RISCV_MODE)
    add_definitions(-DNUM_CORES=2)
    add_definitions(-DSRAM_SIZE=0x84000)      # 520KB
    add_definitions(-DGPIO_PINS=48)
    add_definitions(-DPIO_BLOCKS=3)
    
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=rv32imafc -mcmodel=medany")
    
else()
    message(FATAL_ERROR "Unknown target: ${BITWISE_TARGET}")
endif()

# Compilation flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Wshadow")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG=1 --coverage")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG=1")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Flags: ${CMAKE_C_FLAGS}")

# Create main library
add_library(bitn STATIC ${ALL_SOURCES})
target_include_directories(bitn PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Create executable for standalone emulator
add_executable(bitn-emu src/main.c)
target_link_libraries(bitn-emu bitn m)

# Unit tests
enable_testing()

# Test: ARM Decoder
if(BITWISE_TARGET MATCHES "RP2040|RP2350_ARM")
    add_executable(test_arm_decoder tests/unit/arm_decoder_test.c)
    target_link_libraries(test_arm_decoder bitn)
    add_test(NAME test_arm_decoder COMMAND test_arm_decoder)
endif()

# Test: RISC-V Decoder
if(BITWISE_TARGET MATCHES "RP2350_RISCV")
    add_executable(test_riscv_decoder tests/unit/riscv_decoder_test.c)
    target_link_libraries(test_riscv_decoder bitn)
    add_test(NAME test_riscv_decoder COMMAND test_riscv_decoder)
endif()

# Test: GPIO
add_executable(test_gpio tests/integration/gpio_test.c)
target_link_libraries(test_gpio bitn)
add_test(NAME test_gpio COMMAND test_gpio)

# Test: UART
add_executable(test_uart tests/integration/uart_test.c)
target_link_libraries(test_uart bitn)
add_test(NAME test_uart COMMAND test_uart)

# Test: Memory
add_executable(test_memory tests/unit/memory_test.c)
target_link_libraries(test_memory bitn)
add_test(NAME test_memory COMMAND test_memory)

# Coverage report (if Debug build)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    find_program(GCOV_PATH gcov)
    find_program(LCOV_PATH lcov)
    
    if(GCOV_PATH AND LCOV_PATH)
        add_custom_target(coverage
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage.info
            COMMAND genhtml coverage.info --output-directory coverage_report
            DEPENDS bitn
        )
    endif()
endif()

# Installation
install(TARGETS bitn bitn-emu
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include/bitn)

# Summary
message(STATUS "")
message(STATUS "====== Build Configuration Summary ======")
message(STATUS "Target: ${BITWISE_TARGET}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "")
message(STATUS "Build output will be in: ${CMAKE_BINARY_DIR}")
message(STATUS "Executable: bitn-emu")
message(STATUS "Library: libbitn.a")
message(STATUS "=========================================")

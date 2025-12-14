cmake_minimum_required(VERSION 3.10)
project(bitN C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter")

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
    src/main.c
    src/token.c
    src/lexer.c
    src/ast.c
    src/parser.c
)

add_executable(bitN ${SOURCES})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-O2")

install(TARGETS bitN DESTINATION bin)
enable_testing()
add_test(NAME lexer_test COMMAND bitN -c "fn main() -> u32 { return 42; }")

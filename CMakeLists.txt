cmake_minimum_required(VERSION 3.10)

# Project name and language
project(GPR_CPU_Emulator LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: Build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Add executable
add_executable(gpr_emulator
    main.cpp
    gpr_cpu.cpp
    assembler.cpp
)

# Include current directory for headers
target_include_directories(gpr_emulator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Optional: Enable warnings
if(MSVC)
    target_compile_options(gpr_emulator PRIVATE /W4 /permissive-)
else()
    target_compile_options(gpr_emulator PRIVATE -Wall -Wextra -pedantic)
endif()

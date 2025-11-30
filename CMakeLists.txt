cmake_minimum_required(VERSION 3.10)
project(AsyncLogger)

# Check C++ compiler and version
message(STATUS "Checking C++ compiler...")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} --version
        OUTPUT_VARIABLE GCC_VERSION_OUTPUT
    )
    string(REGEX MATCH "[0-9]+\.[0-9]+\.[0-9]+" GCC_VERSION ${GCC_VERSION_OUTPUT})
    message(STATUS "GCC version: ${GCC_VERSION}")
    
    if(GCC_VERSION VERSION_LESS "5.0.0")
        message(FATAL_ERROR "GCC version must be at least 5.0.0")
    endif()
else()
    message(FATAL_ERROR "Only GCC compiler is supported")
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compile options
add_compile_options(-Wall -Wextra -O2)

# Include directories
include_directories(${PROJECT_SOURCE_DIR})

# Source files
set(SOURCE_FILES
    AsyncLogger.cpp
    LogFile.cpp
    Logger.cpp
    main.cpp
)

# Create executable
add_executable(AsyncLogger ${SOURCE_FILES})

# Link libraries
target_link_libraries(AsyncLogger pthread)

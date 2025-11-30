cmake_minimum_required(VERSION 3.10)
project(VectorSimilaritySearch)

# Set C++ standard to C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable optimizations
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")

# Add executable target
add_executable(vector_search main.cpp VectorDB.cpp)

# Include directories
target_include_directories(vector_search PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

cmake_minimum_required(VERSION 3.1)
project(IntegerCodec)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the source files
set(SOURCES
    IntegerCodec.cpp
    main.cpp
)

# Add the header files
set(HEADERS
    IntegerCodec.h
)

# Create the executable
add_executable(IntegerCodec ${SOURCES} ${HEADERS})

# Set compiler options
if(MSVC)
    target_compile_options(IntegerCodec PRIVATE /W4 /O2)
else()
    target_compile_options(IntegerCodec PRIVATE -Wall -Wextra -O2)
endif()

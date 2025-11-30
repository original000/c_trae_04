cmake_minimum_required(VERSION 3.10)
project(threadpool)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 检测操作系统
if(WIN32)
    add_definitions(-D_WIN32)
elseif(UNIX)
    add_definitions(-D_UNIX)
endif()

# 检测编译器版本
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_definitions(-DGCC_VERSION_MAJOR=${CMAKE_CXX_COMPILER_VERSION_MAJOR})
    add_definitions(-DGCC_VERSION_MINOR=${CMAKE_CXX_COMPILER_VERSION_MINOR})
    add_definitions(-DGCC_VERSION_PATCH=${CMAKE_CXX_COMPILER_VERSION_PATCH})
endif()

# 检测C++14支持
if(CMAKE_CXX_STANDARD GREATER_EQUAL 14)
    add_definitions(-DCPP14_SUPPORTED)
endif()

# 添加可执行文件
add_executable(threadpool main.cpp ThreadPool.cpp LockFreeQueue.h)

# 链接必要的库
if(WIN32)
    target_link_libraries(threadpool pthread)
elseif(UNIX)
    target_link_libraries(threadpool pthread)
endif()

cmake_minimum_required(VERSION 3.10)

project(CoroutineScheduler)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找所有源文件
file(GLOB SOURCES "*.cpp")

# 生成可执行文件
add_executable(coroutine_scheduler ${SOURCES})

# 链接Windows库
target_link_libraries(coroutine_scheduler kernel32)

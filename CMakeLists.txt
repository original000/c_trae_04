cmake_minimum_required(VERSION 3.10)
project(RingBufferTest)

# 设置C++标准为C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找所有源文件
file(GLOB SOURCES "*.cpp")

# 添加可执行文件
add_executable(RingBufferTest ${SOURCES})

# 链接必要的库
target_link_libraries(RingBufferTest pthread)

# 设置编译选项
if(MSVC)
    # Visual Studio编译选项
    target_compile_options(RingBufferTest PRIVATE /W4 /O2)
else()
    # GCC/Clang编译选项
    target_compile_options(RingBufferTest PRIVATE -Wall -Wextra -O3 -march=native)
endif()
cmake_minimum_required(VERSION 3.10)

project(ZeroCopyParser)

# 设置C++标准为C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找g++编译器
find_program(GCC_EXECUTABLE g++)
if(GCC_EXECUTABLE)
    message(STATUS "找到g++编译器: ${GCC_EXECUTABLE}")
else()
    message(FATAL_ERROR "未找到g++编译器")
endif()

# 添加编译选项
add_compile_options(
    -O2  # 优化级别
    -Wall  # 开启所有警告
    -Wextra  # 开启额外警告
    -pedantic  # 严格遵循C++标准
)

# 添加可执行文件
add_executable(parser main.cpp)

# 链接必要的库
# 对于Windows下的MinGW，可能需要链接一些额外的库
if(WIN32)
    target_link_libraries(parser PRIVATE ws2_32)
endif()

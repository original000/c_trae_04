cmake_minimum_required(VERSION 3.10)

# 项目名称
project(ObjectPool)

# 设置C++标准为C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找所有源文件
file(GLOB SOURCES "*.cpp")

# 创建可执行文件
add_executable(ObjectPool ${SOURCES})

# 链接必要的库
target_link_libraries(ObjectPool pthread)

# 设置编译选项
if(MSVC)
    # Visual Studio编译选项
    target_compile_options(ObjectPool PRIVATE /W4 /O2)
else()
    # GCC或Clang编译选项
    target_compile_options(ObjectPool PRIVATE -Wall -Wextra -O2)
endif()

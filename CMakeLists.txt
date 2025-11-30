cmake_minimum_required(VERSION 3.10)
project(MiniRedis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加httplib库
include_directories(include)

# 源文件
file(GLOB SOURCES "src/*.cpp" "main.cpp")

# 可执行文件
add_executable(MiniRedis ${SOURCES})

# 测试程序
add_executable(MiniRedisTest test.cpp src/Database.cpp)

# 链接库
if(WIN32)
    target_link_libraries(MiniRedis ws2_32)
    target_link_libraries(MiniRedisTest ws2_32)
endif()

# 包含目录
include_directories(src)

# 安装目标
install(TARGETS MiniRedis DESTINATION bin)
install(TARGETS MiniRedisTest DESTINATION bin)

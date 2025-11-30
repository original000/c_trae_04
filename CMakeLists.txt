cmake_minimum_required(VERSION 3.10)
project(SnowflakeIDGenerator)

# 检查操作系统
if(WIN32)
    message(STATUS "Operating System: Windows")
elseif(UNIX)
    message(STATUS "Operating System: Unix/Linux")
else()
    message(STATUS "Operating System: Unknown")
endif()

# 检查g++版本
execute_process(
    COMMAND g++ --version
    OUTPUT_VARIABLE GXX_VERSION_OUTPUT
)
message(STATUS "g++ Version: ${GXX_VERSION_OUTPUT}")

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找Google Test
find_package(GTest)
if(NOT GTEST_FOUND)
    message(WARNING "Google Test not found. Skipping test compilation.")
endif()

# 包含目录
include_directories(${GTEST_INCLUDE_DIRS} src)

# 源文件
set(SOURCE_FILES
    src/Snowflake.cpp
    src/HttpServer.cpp
    src/Config.cpp
    main.cpp
)

# 生成可执行文件
add_executable(SnowflakeIDGenerator ${SOURCE_FILES})

# 链接库
if(WIN32)
    target_link_libraries(SnowflakeIDGenerator ws2_32)
endif()

# 安装目标
install(TARGETS SnowflakeIDGenerator DESTINATION bin)

# 如果找到Google Test库，则编译测试程序
if(GTEST_FOUND)
    # 测试文件
    set(TEST_FILES
        test/SnowflakeTest.cpp
    )

    # 生成测试可执行文件
    add_executable(SnowflakeTest ${TEST_FILES} src/Snowflake.cpp src/Config.cpp)

    # 链接库
    target_link_libraries(SnowflakeTest ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

    # 启用测试
    enable_testing()
    add_test(NAME SnowflakeTest COMMAND SnowflakeTest)
endif()

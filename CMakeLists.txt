cmake_minimum_required(VERSION 3.0)
project(MemLeakDetector)

# 设置C++标准为C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(MemLeakDetectorTest main.cpp)
add_executable(SimpleMemLeakDetectorTest simple_test.cpp)
add_executable(UltraSimpleMemLeakDetectorTest ultra_simple_test.cpp)
add_executable(TestSimpleDetector test_simple_detector.cpp)

# 链接必要的库
target_link_libraries(MemLeakDetectorTest dbghelp)
target_link_libraries(SimpleMemLeakDetectorTest dbghelp)
target_link_libraries(UltraSimpleMemLeakDetectorTest dbghelp)
target_link_libraries(TestSimpleDetector dbghelp)

# 设置编译选项
if(MSVC)
    target_compile_options(MemLeakDetectorTest PRIVATE /W4 /WX)
    target_compile_options(SimpleMemLeakDetectorTest PRIVATE /W4 /WX)
    target_compile_options(UltraSimpleMemLeakDetectorTest PRIVATE /W4 /WX)
    target_compile_options(TestSimpleDetector PRIVATE /W4 /WX)
else()
    target_compile_options(MemLeakDetectorTest PRIVATE -Wall -Wextra -Werror)
    target_compile_options(SimpleMemLeakDetectorTest PRIVATE -Wall -Wextra -Werror)
    target_compile_options(UltraSimpleMemLeakDetectorTest PRIVATE -Wall -Wextra -Werror)
    target_compile_options(TestSimpleDetector PRIVATE -Wall -Wextra -Werror)
endif()

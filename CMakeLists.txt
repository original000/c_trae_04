cmake_minimum_required(VERSION 3.10)
project(shortener VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find SQLite3
find_package(SQLite3 REQUIRED)

# Include directories
include_directories(${SQLite3_INCLUDE_DIRS})
include_directories(src)
include_directories(src/server)
include_directories(src/service)
include_directories(src/storage)
include_directories(src/cache)
include_directories(src/model)
include_directories(src/utils)

# Source files
set(SOURCES
    main.cpp
    src/server/HttpServer.cpp
    src/service/ShortenService.cpp
    src/storage/SqliteStorage.cpp
    src/utils/Logger.cpp
    src/cache/LRUCache.h
    src/model/ShortLink.h
    src/model/Stats.h
    src/utils/Logger.h
    src/utils/Config.h
)

# Executable
add_executable(shortener ${SOURCES})

# Link libraries
target_link_libraries(shortener ${SQLite3_LIBRARIES} pthread ws2_32)

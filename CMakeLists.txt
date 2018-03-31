cmake_minimum_required (VERSION 2.6)
project (natpunch)

set (natpunch_VERSION_MAJOR 1)
set (natpunch_VERSION_MINOR 0)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if (CMAKE_BUILD_TYPE MATCHES Release)
    add_definitions(-DNDEBUG)
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra -std=c11 -pedantic")
set(CMAKE_C_FLAGS_DEBUG "-ggdb -O0")
set(CMAKE_C_FLAGS_RELEASE "-O3 -march=native -flto")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


add_definitions(-D_POSIX_C_SOURCE=200809L)
add_definitions(-D_DEFAULT_SOURCE)

include_directories(src)
file(GLOB MAIN
    "src/*.c"
    "src/*.h"
    "src/wrappers/*.c"
    "src/wrappers/*.h"
    )

add_executable(natpunch ${MAIN})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(natpunch rt Threads::Threads)

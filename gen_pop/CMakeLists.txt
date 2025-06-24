cmake_minimum_required(VERSION 3.20)
project(gen_pop LANGUAGES CXX)

# Compiler Settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS   OFF)

# Default to Release unless user passes -DCMAKE_BUILD_TYPE=Debug
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Release or Debug" FORCE)
endif()

# Source Files
file(GLOB PROJECT_SRC
     src/*.cpp
     third_party/duckdb.cpp)    # the big amalgamation

add_executable(gen_pop ${PROJECT_SRC})

target_include_directories(gen_pop
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include)

target_include_directories(gen_pop SYSTEM
    PRIVATE
        ${PROJECT_SOURCE_DIR}/third_party)

# Silence duckdb.cpp warnings
set_source_files_properties(
    third_party/duckdb.cpp
    PROPERTIES COMPILE_FLAGS "-w")

# Required for duckdb
target_link_libraries(gen_pop PRIVATE dl z pthread)

target_compile_options(gen_pop PRIVATE
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>)

cmake_minimum_required(VERSION 3.28)

# ---- Project ----

# Note: update this to your new project's name and version
project(
  Taskmaster
  VERSION 1.0
  LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
find_package(Readline REQUIRED)

include(FetchContent)
include(CTest)

add_subdirectory(source)
add_subdirectory(test)

enable_testing()

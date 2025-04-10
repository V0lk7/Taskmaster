cmake_minimum_required(VERSION 3.21)

# ---- Project ----

# Note: update this to your new project's name and version
project(
  Taskmaster
  VERSION 1.0
  LANGUAGES CXX C
)

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests_client)

target_link_libraries(tests_client PRIVATE Catch2::Catch2WithMain)

target_compile_options(
  tests_client
  PRIVATE
    -gdwarf
)

target_include_directories(tests_client PRIVATE ${CMAKE_SOURCE_DIR}/include/)
target_include_directories(tests_client PRIVATE ${catch2_SOURCE_DIR}/src/)

file(GLOB_RECURSE SOURCES_TESTS "${CMAKE_CURRENT_SOURCE_DIR}/client/*.cpp")

target_sources(tests_client PRIVATE ${SOURCES_TESTS})

include(CTest)
include(Catch)
catch_discover_tests(tests_client)

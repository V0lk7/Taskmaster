cmake_minimum_required(VERSION 3.31)

project(
  Taskmaster
  VERSION 1.0
  LANGUAGES CXX C
)

add_executable(taskmasterctl)
add_executable(taskmasterd)

#==================Shared library

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_library(TESTLIB NAMES yaml-cpp PATHS "${CMAKE_SOURCE_DIR}/lib")

if(NOT TESTLIB)
  message(FATAL_ERROR "yaml-cpp library not found!")
endif()

add_library(libyaml_cpp SHARED IMPORTED)
set_target_properties(libyaml_cpp PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libyaml-cpp.so)

#==================Binaries
target_compile_features(taskmasterctl PUBLIC cxx_std_11)
target_compile_features(taskmasterd PUBLIC cxx_std_11)

target_compile_options(taskmasterctl
  PRIVATE
    -Wall
    -Wextra
    -Werror
)
target_compile_options(taskmasterd
  PRIVATE
    -Wall
    -Wextra
    -Werror
)

target_include_directories(taskmasterctl
  PRIVATE 
    ${CMAKE_SOURCE_DIR}/include/taskmasterctl
    ${CMAKE_SOURCE_DIR}/include/yaml-cpp-0.8.0
)

target_include_directories(taskmasterd
  PRIVATE 
    ${CMAKE_SOURCE_DIR}/include/taskmasterd
    ${CMAKE_SOURCE_DIR}/include/yaml-cpp-0.8.0
)

file(GLOB_RECURSE TASKMASTERCTL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/taskmasterctl/*.cpp")
file(GLOB_RECURSE TASKMASTERD_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/taskmasterd/*.cpp")

target_sources(taskmasterctl PRIVATE ${TASKMASTERCTL_SOURCES})
target_sources(taskmasterd PRIVATE ${TASKMASTERD_SOURCES})

target_link_libraries(taskmasterctl PRIVATE libyaml_cpp)
target_link_libraries(taskmasterd PRIVATE libyaml_cpp)

install(TARGETS taskmasterd taskmasterctl DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
install(FILES ${CMAKE_SOURCE_DIR}/lib/libyaml-cpp.so DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})


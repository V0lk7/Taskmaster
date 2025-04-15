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

find_library(TESTLIB NAMES yaml-cpp nanomsg PATHS "${CMAKE_SOURCE_DIR}/lib")

if(NOT TESTLIB)
  message(FATAL_ERROR "Library not found!")
endif()

add_library(libyaml_cpp SHARED IMPORTED)
set_target_properties(
  libyaml_cpp PROPERTIES
  IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/yaml-cpp/libyaml-cpp.so.0.8.0
)

add_library(libnanomsg SHARED IMPORTED)
set_target_properties(
  libnanomsg
  PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/nanomsg/libnanomsg.so.6.0.1
)

#==================Static lib
file(GLOB_RECURSE COMMON_SOURCES
  "${CMAKE_SOURCE_DIR}/source/common/*.cpp"
)

add_library(common STATIC ${COMMON_SOURCES})

target_compile_options(
  common
  PRIVATE
    -Wall
    -Wextra
    -Werror
    -gdwarf
)

target_include_directories(common PRIVATE "${CMAKE_SOURCE_DIR}/include/")

#==================Binaries
target_compile_features(taskmasterctl PUBLIC cxx_std_11)
target_compile_features(taskmasterd PUBLIC cxx_std_11)

target_compile_options(
  taskmasterctl
  PRIVATE
    -Wall
    -Wextra
    -Werror
    -gdwarf
)
target_compile_options(
  taskmasterd
  PRIVATE
    -Wall
    -Wextra
    -Werror
)

target_include_directories(
  taskmasterctl
  PRIVATE 
    ${CMAKE_SOURCE_DIR}/include/
    ${CMAKE_SOURCE_DIR}/include/yaml-cpp
    ${CMAKE_SOURCE_DIR}/include/libnanomsg
)

target_include_directories(
  taskmasterd
  PRIVATE 
    ${CMAKE_SOURCE_DIR}/include/
    ${CMAKE_SOURCE_DIR}/include/yaml-cpp-0.8.0
    ${CMAKE_SOURCE_DIR}/include/libnanomsg.6.0.1
)

file(GLOB_RECURSE TASKMASTERCTL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/client/*.cpp")
file(GLOB_RECURSE TASKMASTERD_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/daemon/*.cpp")

target_sources(taskmasterctl PRIVATE ${TASKMASTERCTL_SOURCES})
target_sources(taskmasterd PRIVATE ${TASKMASTERD_SOURCES})

target_link_libraries(taskmasterctl PRIVATE common libyaml_cpp libnanomsg Readline::Readline)
target_link_libraries(taskmasterd PRIVATE common libyaml_cpp libnanomsg Readline::Readline)

install(TARGETS taskmasterd taskmasterctl DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
install(
  FILES 
  ${CMAKE_SOURCE_DIR}/lib/yaml-cpp/libyaml-cpp.so.0.8.0
  ${CMAKE_SOURCE_DIR}/lib/nanomsg/libnanomsg.so.6.0.1
  DESTINATION
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)


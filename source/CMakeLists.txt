cmake_minimum_required(VERSION 3.28)

project(
  Taskmaster
  VERSION 1.0
  LANGUAGES CXX C
)

add_executable(taskmasterctl)
add_executable(taskmasterd)

# Set the directories for the symlinks
set(NANOMSG_LIB_DIR ${CMAKE_SOURCE_DIR}/lib/nanomsg)
set(YAML_CPP_LIB_DIR ${CMAKE_SOURCE_DIR}/lib/yaml-cpp)

# Function to create symlinks if not exist
function(create_symlink TARGET LINK_NAME)
    if(NOT EXISTS ${LINK_NAME})
        message(STATUS "Creating symlink: ${LINK_NAME} -> ${TARGET}")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink ${TARGET} ${LINK_NAME}
            RESULT_VARIABLE symlink_result
        )
        if(symlink_result)
            message(FATAL_ERROR "Failed to create symlink: ${TARGET} -> ${LINK_NAME}")
        endif()
    else()
        message(STATUS "Symlink already exists: ${LINK_NAME}")
    endif()
endfunction()

# Create symlinks for nanomsg if not already created
create_symlink(${NANOMSG_LIB_DIR}/libnanomsg.so.6.0.1 ${NANOMSG_LIB_DIR}/libnanomsg.so)
create_symlink(${NANOMSG_LIB_DIR}/libnanomsg.so ${NANOMSG_LIB_DIR}/libnanomsg.so.6)

# Create symlinks for yaml-cpp if not already created
create_symlink(${YAML_CPP_LIB_DIR}/libyaml-cpp.so.0.8.0 ${YAML_CPP_LIB_DIR}/libyaml-cpp.so)
create_symlink(${YAML_CPP_LIB_DIR}/libyaml-cpp.so ${YAML_CPP_LIB_DIR}/libyaml-cpp.so.0.8)

set_target_properties(taskmasterctl taskmasterd PROPERTIES
  BUILD_WITH_INSTALL_RPATH TRUE
  BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib/yaml-cpp;${CMAKE_SOURCE_DIR}/lib/nanomsg"
  INSTALL_RPATH "${CMAKE_SOURCE_DIR}/lib/yaml-cpp;${CMAKE_SOURCE_DIR}/lib/nanomsg"
  SKIP_BUILD_RPATH FALSE
)

#==================Shared library

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message(DEBUG "${CMAKE_SOURCE_DIR}")

find_library(YAMLCPP_LIB NAMES yaml-cpp PATHS "${CMAKE_SOURCE_DIR}/lib/yaml-cpp/")
find_library(NANOMSG_LIB NAMES nanomsg PATHS "${CMAKE_SOURCE_DIR}/lib/nanomsg/")

if(NOT YAMLCPP_LIB)
  message(FATAL_ERROR "|${CMAKE_SOURCE_DIR}|yaml-cpp library not found!")
endif()

if(NOT NANOMSG_LIB)
  message(FATAL_ERROR "nanomsg library not found!")
endif()

add_library(libyaml_cpp SHARED IMPORTED)
set_target_properties(
  libyaml_cpp PROPERTIES
  IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/yaml-cpp/libyaml-cpp.so
)

add_library(libnanomsg SHARED IMPORTED)
set_target_properties(
  libnanomsg
  PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/nanomsg/libnanomsg.so
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
	-gdwarf
)

target_include_directories(
  taskmasterctl
  PRIVATE 
    ${CMAKE_SOURCE_DIR}/include/
)

target_include_directories(
  taskmasterd
  PRIVATE
    ${CMAKE_SOURCE_DIR}/include/taskmasterd
    ${CMAKE_SOURCE_DIR}/include/
)

file(GLOB_RECURSE TASKMASTERCTL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/client/*.cpp")
file(GLOB_RECURSE TASKMASTERD_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/daemon/*.cpp")

target_sources(taskmasterctl PRIVATE ${TASKMASTERCTL_SOURCES})
target_sources(taskmasterd PRIVATE ${TASKMASTERD_SOURCES})

target_link_libraries(taskmasterctl PRIVATE common libyaml_cpp libnanomsg Readline::Readline)
target_link_libraries(taskmasterd PRIVATE common libyaml_cpp libnanomsg Readline::Readline)

target_precompile_headers(taskmasterctl PRIVATE "${CMAKE_SOURCE_DIR}/include/pch.hpp")

install(TARGETS taskmasterd taskmasterctl DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
install(
  FILES
  ${CMAKE_SOURCE_DIR}/lib/yaml-cpp/libyaml-cpp.so.0.8.0
  ${CMAKE_SOURCE_DIR}/lib/yaml-cpp/libyaml-cpp.so.0.8
  ${CMAKE_SOURCE_DIR}/lib/yaml-cpp/libyaml-cpp.so
  ${CMAKE_SOURCE_DIR}/lib/nanomsg/libnanomsg.so.6.0.1
  ${CMAKE_SOURCE_DIR}/lib/nanomsg/libnanomsg.so.6
  ${CMAKE_SOURCE_DIR}/lib/nanomsg/libnanomsg.so
  DESTINATION
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)


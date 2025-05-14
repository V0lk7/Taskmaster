cmake_minimum_required(VERSION 3.21)

# ---- Project ----

# Note: update this to your new project's name and version
project(
  Taskmaster
  VERSION 1.0
  LANGUAGES CXX C
)

add_executable(tests_client)

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

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

file(GLOB_RECURSE MYLIB_SOURCES
  "${CMAKE_SOURCE_DIR}/source/common/*.cpp"
  "${CMAKE_SOURCE_DIR}/source/client/Client/*.cpp"
  "${CMAKE_SOURCE_DIR}/source/client/Client/Console/*.cpp"
  "${CMAKE_SOURCE_DIR}/source/client/Client/Epoll/*.cpp"
)

add_library(myLib STATIC ${MYLIB_SOURCES})
target_link_libraries(myLib PRIVATE libyaml_cpp libnanomsg Readline::Readline)
target_include_directories(myLib PUBLIC ${CMAKE_SOURCE_DIR}/include/)


set_target_properties(tests_client PROPERTIES
  BUILD_WITH_INSTALL_RPATH TRUE
  BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib/yaml-cpp;${CMAKE_SOURCE_DIR}/lib/nanomsg"
  INSTALL_RPATH "${CMAKE_SOURCE_DIR}/lib/yaml-cpp;${CMAKE_SOURCE_DIR}/lib/nanomsg"
  SKIP_BUILD_RPATH FALSE
)

target_link_libraries(tests_client PRIVATE myLib Catch2::Catch2WithMain)

target_compile_options(
  tests_client
  PRIVATE
    -gdwarf
)

target_include_directories(tests_client PRIVATE ${CMAKE_SOURCE_DIR}/include/)
target_include_directories(tests_client PRIVATE ${catch2_SOURCE_DIR}/src/)

file(GLOB_RECURSE SOURCES_TESTS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

target_sources(tests_client PRIVATE ${SOURCES_TESTS})


# Optional: print to verify
message(STATUS "BUILD_RPATH: $<TARGET_PROPERTY:tests_client,BUILD_RPATH>")

include(CTest)
include(Catch)
catch_discover_tests(tests_client)

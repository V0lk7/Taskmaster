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
  "${CMAKE_SOURCE_DIR}/source/daemon/parsing/*.cpp"
  "${CMAKE_SOURCE_DIR}/source/daemon/*.cpp"
)

list(REMOVE_ITEM MYLIB_SOURCES "${CMAKE_SOURCE_DIR}/source/daemon/main.cpp")

add_library(myLib STATIC ${MYLIB_SOURCES})
target_link_libraries(myLib PRIVATE libyaml_cpp libnanomsg Readline::Readline)
target_include_directories(myLib PUBLIC ${CMAKE_SOURCE_DIR}/include/)

add_executable(tests)

set_target_properties(tests PROPERTIES
  BUILD_WITH_INSTALL_RPATH TRUE
  BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib/yaml-cpp;${CMAKE_SOURCE_DIR}/lib/nanomsg"
  INSTALL_RPATH "${CMAKE_SOURCE_DIR}/lib/yaml-cpp;${CMAKE_SOURCE_DIR}/lib/nanomsg"
  SKIP_BUILD_RPATH FALSE
)

target_link_libraries(tests PRIVATE myLib Catch2::Catch2WithMain)

target_compile_options(
  tests
  PRIVATE
    -gdwarf
)

target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/include/)
target_include_directories(tests PRIVATE ${catch2_SOURCE_DIR}/src/)

file(GLOB_RECURSE SOURCES_TESTS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

target_sources(tests PRIVATE ${SOURCES_TESTS})

# Définit la sortie du binaire
set_target_properties(tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test
)

# Récupère la valeur exacte du dossier binaire
get_target_property(TEST_BIN_DIR tests RUNTIME_OUTPUT_DIRECTORY)

# Crée le dossier logs dans ce dossier
file(MAKE_DIRECTORY "${TEST_BIN_DIR}/logs")


include(CTest)
include(Catch)
catch_discover_tests(tests)

# TODO : !! Works for Ubuntu, have to check for Arch
cmake_minimum_required(VERSION 3.10)
project(ZC VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
include_directories(SYSTEM external)

# Find LLVM and Clang (LLVM added for Ubuntu)
find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Using ClangConfig.cmake in: ${Clang_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${Clang_INCLUDE_DIRS})

# find_library(CLANG_LIB NAMES clang clang-18)
# find_path(CLANG_INCLUDE_DIRS NAMES clang-c/Index.h PATH_SUFFIXES llvm-18/include clang)

# if(NOT CLANG_LIB OR NOT CLANG_INCLUDE_DIRS)
#   message(FATAL_ERROR "Clang library or include directories not found.")
# endif()
#
# message(STATUS "Found libclang: ${CLANG_LIB}")
# message(STATUS "Found clang headers: ${CLANG_INCLUDE_DIR}")

# Add compilation flags
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND "${LLVM_DEFINITIONS}")
add_definitions(${LLVM_DEFINITIONS_LIST})

# Fetch json parsing library
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

# Source code files
file(GLOB SOURCES "src/*.cc" "src/*.c")

# Create the executable
add_executable(zc ${SOURCES})

# Link libraries
target_include_directories(zc PRIVATE ${CLANG_INCLUDE_DIRS})
target_link_libraries(zc PRIVATE
  nlohmann_json::nlohmann_json
  ${CLANG_LIB}
  LLVM
)

# Installation rules
install(TARGETS zc DESTINATION bin)
install(FILES etc/config.json DESTINATION /etc/zc)

# TODO : !! Works for Ubuntu, have to check for Arch
cmake_minimum_required(VERSION 3.12)
project(ZC VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependencies ---

# 1. Fetch JSON (Nlohmann)
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

# 2. Find LLVM/Clang
# On cherche d'abord les paquets pour les headers
find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

message(STATUS "LLVM found: ${LLVM_PACKAGE_VERSION}")

include_directories(SYSTEM external)
# Inclusion des headers systèmes de LLVM/Clang
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${Clang_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# 3. Find specifically the library binary (libclang.so / .dylib)
# C'est important car target_link_libraries a besoin du chemin vers le .so
find_library(CLANG_LIB
    NAMES clang libclang clang-18 clang-17 clang-16 clang-15
    HINTS ${LLVM_LIBRARY_DIRS} ${Clang_LIBRARY_DIRS}
)

if(NOT CLANG_LIB)
  message(FATAL_ERROR "Could not find libclang shared library.")
endif()
message(STATUS "Found libclang: ${CLANG_LIB}")

# --- Build ---

# Source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cc")

add_executable(zc ${SOURCES})

# Include directories for your project
target_include_directories(zc PRIVATE include)

# Link
target_link_libraries(zc PRIVATE
  nlohmann_json::nlohmann_json
  ${CLANG_LIB}
  # Parfois nécessaire sous Linux pour LLVM :
  pthread
  dl
  tinfo
)

# --- Installation Rules (INDISPENSABLE pour install.sh) ---

# Installe le binaire dans /usr/local/bin (par défaut)
install(TARGETS zc DESTINATION bin)

# Note: On n'installe PAS la config ici car votre script shell
# gère la config utilisateur dans ~/.zc

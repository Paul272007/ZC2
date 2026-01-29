# TODO : !! Works for Ubuntu, have to check for Arch
cmake_minimum_required(VERSION 3.12)
project(ZC VERSION 1.0 LANGUAGES C CXX)

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

add_executable(zc
  src/commands/Lib/Create.cc
  src/commands/Lib/List.cc
  src/commands/Lib/Remove.cc
  src/commands/Build.cc
  src/commands/Init.cc
  src/commands/Project.cc
  src/commands/Run.cc
  src/objects/File.cc
  src/objects/ProjectsRegistry.cc
  src/objects/Registry.cc
  src/objects/Settings.cc
  src/objects/ZCError.cc
  src/helpers.cc
  src/main.cc
  src/zcio.cc
)

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

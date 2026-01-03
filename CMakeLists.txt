cmake_minimum_required(VERSION 3.10)

project(ZC VERSION 1.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

include_directories(SYSTEM external)

file(GLOB SOURCES "src/*.cc")

include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

add_executable(zc ${SOURCES})

target_link_libraries(zc PRIVATE nlohmann_json::nlohmann_json)

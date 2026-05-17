cmake_minimum_required(VERSION 3.15)
project(Linear CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# nlohmann/json — header-only JSON library
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# Catch2 — unit-test framework
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)

# ── FileUtilities library ─────────────────────────────────────────────────────
add_library(file_utilities
    src/FileUtilities.cpp
)
target_include_directories(file_utilities PUBLIC include)
target_link_libraries(file_utilities PUBLIC nlohmann_json::nlohmann_json)

# ── Tests ─────────────────────────────────────────────────────────────────────
enable_testing()

add_executable(test_file_utilities
    tests/test_file_utilities.cpp
)
target_link_libraries(test_file_utilities PRIVATE file_utilities Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(test_file_utilities)

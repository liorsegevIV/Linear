cmake_minimum_required(VERSION 3.14)
project(LinearMath CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(mathutils src/math.cpp)
target_include_directories(mathutils PUBLIC include)

add_executable(math_smoke_test tests/main.cpp)
target_link_libraries(math_smoke_test PRIVATE mathutils)

cmake_minimum_required(VERSION 3.16)
project(actually_smart_pointer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(llama.cpp)
include_directories(llama.cpp)

include_directories(include)

add_library(actually_smart_pointer STATIC
    src/actually_smart_pointer.cpp
)
target_link_libraries(actually_smart_pointer PRIVATE llama)

add_executable(example main.cpp)
target_link_libraries(example PRIVATE actually_smart_pointer)
include_directories(include)


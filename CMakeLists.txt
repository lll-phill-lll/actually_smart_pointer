cmake_minimum_required(VERSION 3.16)
project(actually_smart_pointer)

option(LLM_DEBUG_LOG "Enable debug logging for LLM prompts and responses" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(CREATE_LINK
  "${CMAKE_BINARY_DIR}/compile_commands.json"
  "${CMAKE_SOURCE_DIR}/compile_commands.json"
  SYMBOLIC
)

add_compile_definitions(DEFAULT_MODEL_PATH="${CMAKE_SOURCE_DIR}/models/deepseek-coder-6.7b-instruct.Q4_K_M.gguf")

add_subdirectory(llama.cpp)
include_directories(llama.cpp)

include_directories(include)

add_library(actually_smart_pointer STATIC
    src/actually_smart_pointer.cpp
)

if (LLM_DEBUG_LOG)
    target_compile_definitions(actually_smart_pointer PRIVATE LLM_DEBUG_LOG_ENABLED)
endif()

target_link_libraries(actually_smart_pointer PRIVATE llama)

add_executable(example main.cpp)
target_link_libraries(example PRIVATE actually_smart_pointer)
include_directories(include)


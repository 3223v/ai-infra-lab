# ============================================================================
# TinyInfer — 轻量级 LLM 推理 Runtime
# C++20 · CPU 优先 → CUDA 加速 · 目标：RTX 3050 4GB 上跑通 <1B 模型
# ============================================================================
cmake_minimum_required(VERSION 3.20)
project(tinyinfer
    VERSION 0.1.0
    DESCRIPTION "Lightweight LLM inference runtime"
    LANGUAGES CXX
)

# ---- C++ 标准 ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- 公共对象库：源码编译一次，CLI 和测试共享 ----
add_library(tinyinfer_lib OBJECT
    src/tensor.cpp
    src/status.cpp
    src/model_config.cpp
    src/cli.cpp
)

target_include_directories(tinyinfer_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(tinyinfer_lib PRIVATE
    -Wall -Wextra -Wpedantic
)

# ---- CLI 可执行文件 ----
add_executable(tinyinfer_cli
    src/cli_main.cpp
)

target_link_libraries(tinyinfer_cli PRIVATE tinyinfer_lib)

target_compile_options(tinyinfer_cli PRIVATE
    -Wall -Wextra -Wpedantic
)

# ---- 测试 ----
enable_testing()

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)
FetchContent_MakeAvailable(Catch2)

add_executable(tinyinfer_tests
    tests/test_tensor.cpp
    tests/test_model_config.cpp
)

target_link_libraries(tinyinfer_tests PRIVATE
    tinyinfer_lib
    Catch2::Catch2WithMain
)

target_compile_options(tinyinfer_tests PRIVATE
    -Wall -Wextra -Wpedantic
)

add_test(NAME tinyinfer_tests COMMAND tinyinfer_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

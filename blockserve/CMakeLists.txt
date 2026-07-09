# ---- 公共对象库：源码只编译一次，主程序和测试共享 ----
add_library(blockserve_lib OBJECT
    src/request.cpp
    src/request_queue.cpp
    src/workload_loader.cpp
    src/simulator.cpp
    src/log.cpp
)

target_include_directories(blockserve_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(blockserve_lib PRIVATE
    -Wall -Wextra -Wpedantic
)

# ---- 主可执行文件 ----
add_executable(blockserve src/main.cpp)
target_link_libraries(blockserve PRIVATE blockserve_lib)

# ---- 测试（Catch2 自动下载） ----
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)
FetchContent_MakeAvailable(Catch2)

add_executable(blockserve_tests
    tests/test_request.cpp
    tests/test_request_queue.cpp
    tests/test_workload_loader.cpp
    tests/test_simulator.cpp
)

target_link_libraries(blockserve_tests PRIVATE
    blockserve_lib
    Catch2::Catch2WithMain
)

target_include_directories(blockserve_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(blockserve_tests PRIVATE
    -Wall -Wextra -Wpedantic
)

# ---- 注册测试到 CTest ----
# WORKING_DIRECTORY 设为项目根目录，确保测试代码中的相对路径正确
add_test(NAME blockserve_tests COMMAND blockserve_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

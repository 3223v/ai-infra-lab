add_executable(blockserve
    src/main.cpp
    src/request.cpp
    src/request_queue.cpp
    src/workload_loader.cpp
    src/simulator.cpp
    src/log.cpp
)

target_include_directories(blockserve PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(blockserve PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

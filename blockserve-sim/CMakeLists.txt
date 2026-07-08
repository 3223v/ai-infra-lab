add_executable(blockserve_sim
    src/main.cpp
    src/request.cpp
    src/request_queue.cpp
)

target_include_directories(blockserve_sim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(blockserve_sim PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

cmake_minimum_required(VERSION 3.25)

# ==============================================================================
# TinyInfer — 单机多请求 LLM 推理框架
# ==============================================================================
project(tinyinfer
  VERSION 0.1.0
  DESCRIPTION "A teaching-oriented single-machine LLM inference framework"
  LANGUAGES CXX
)

# ----------------------------------------------------------------------------
# C++20 标准
# ----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 导出编译命令 (for clang-tidy, IDE 等)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ----------------------------------------------------------------------------
# 选项
# ----------------------------------------------------------------------------
option(TINYINFER_BUILD_TESTS   "Build unit and integration tests" ON)
option(TINYINFER_BUILD_BENCH   "Build benchmarks"                OFF)
option(TINYINFER_ENABLE_ASAN   "Enable AddressSanitizer"          OFF)
option(TINYINFER_ENABLE_UBSAN  "Enable UndefinedBehaviorSanitizer" OFF)
option(TINYINFER_ENABLE_TSAN   "Enable ThreadSanitizer"           OFF)
option(TINYINFER_USE_LIBTORCH  "Enable LibTorch backend"          OFF)
option(TINYINFER_USE_DROGON    "Enable Drogon HTTP server"        OFF)

# ----------------------------------------------------------------------------
# 版本信息
# ----------------------------------------------------------------------------
set(TINYINFER_VERSION_MAJOR 0)
set(TINYINFER_VERSION_MINOR 1)
set(TINYINFER_VERSION_PATCH 0)
set(TINYINFER_VERSION "${TINYINFER_VERSION_MAJOR}.${TINYINFER_VERSION_MINOR}.${TINYINFER_VERSION_PATCH}")

# ----------------------------------------------------------------------------
# CMake 模块路径
# ----------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ----------------------------------------------------------------------------
# 依赖管理
# ----------------------------------------------------------------------------
include(cmake/Dependencies.cmake)
include(cmake/CompilerWarnings.cmake)

# 查找 GoogleTest
if(TINYINFER_BUILD_TESTS)
  include(cmake/Sanitizers.cmake)
endif()

# ----------------------------------------------------------------------------
# 源码目录
# ----------------------------------------------------------------------------
set(TINYINFER_INCLUDE_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(TINYINFER_SRC_DIR      "${CMAKE_CURRENT_SOURCE_DIR}/src")

# ============================================================================
# Library: tinyinfer_common — 错误处理、工具类型、时钟
# ============================================================================
add_library(tinyinfer_common INTERFACE)

target_include_directories(tinyinfer_common INTERFACE
  "${TINYINFER_INCLUDE_DIR}"
)

target_compile_features(tinyinfer_common INTERFACE cxx_std_20)

add_library(tinyinfer::common ALIAS tinyinfer_common)

# ============================================================================
# Library: tinyinfer_domain — 请求、序列、状态机、采样参数
# ============================================================================
add_library(tinyinfer_domain INTERFACE)
target_include_directories(tinyinfer_domain INTERFACE "${TINYINFER_INCLUDE_DIR}")
target_link_libraries(tinyinfer_domain INTERFACE tinyinfer::common)
target_compile_features(tinyinfer_domain INTERFACE cxx_std_20)

add_library(tinyinfer::domain ALIAS tinyinfer_domain)

# ============================================================================
# Library: tinyinfer_streaming — OutputChannel, 流式输出
# ============================================================================
add_library(tinyinfer_streaming INTERFACE)
target_include_directories(tinyinfer_streaming INTERFACE "${TINYINFER_INCLUDE_DIR}")
target_link_libraries(tinyinfer_streaming INTERFACE tinyinfer::common tinyinfer::domain)
target_compile_features(tinyinfer_streaming INTERFACE cxx_std_20)

add_library(tinyinfer::streaming ALIAS tinyinfer_streaming)

# ============================================================================
# Library: tinyinfer_cache_core — KV Cache 抽象接口
# ============================================================================
add_library(tinyinfer_cache_core INTERFACE)
target_include_directories(tinyinfer_cache_core INTERFACE "${TINYINFER_INCLUDE_DIR}")
target_link_libraries(tinyinfer_cache_core INTERFACE tinyinfer::common)
target_compile_features(tinyinfer_cache_core INTERFACE cxx_std_20)

add_library(tinyinfer::cache_core ALIAS tinyinfer_cache_core)

# ============================================================================
# Library: tinyinfer_scheduler — 调度器
# ============================================================================
add_library(tinyinfer_scheduler INTERFACE)
target_include_directories(tinyinfer_scheduler INTERFACE "${TINYINFER_INCLUDE_DIR}")
target_link_libraries(tinyinfer_scheduler INTERFACE
  tinyinfer::common
  tinyinfer::domain
  tinyinfer::cache_core
  tinyinfer::streaming
)
target_compile_features(tinyinfer_scheduler INTERFACE cxx_std_20)

add_library(tinyinfer::scheduler ALIAS tinyinfer_scheduler)

# ============================================================================
# Library: tinyinfer_runtime_core — 模型执行抽象
# ============================================================================
add_library(tinyinfer_runtime_core INTERFACE)
target_include_directories(tinyinfer_runtime_core INTERFACE "${TINYINFER_INCLUDE_DIR}")
target_link_libraries(tinyinfer_runtime_core INTERFACE tinyinfer::domain tinyinfer::cache_core)
target_compile_features(tinyinfer_runtime_core INTERFACE cxx_std_20)

add_library(tinyinfer::runtime_core ALIAS tinyinfer_runtime_core)

# ============================================================================
# Library: tinyinfer_model_core — 模型后端抽象
# ============================================================================
add_library(tinyinfer_model_core INTERFACE)
target_include_directories(tinyinfer_model_core INTERFACE "${TINYINFER_INCLUDE_DIR}")
target_link_libraries(tinyinfer_model_core INTERFACE tinyinfer::runtime_core)
target_compile_features(tinyinfer_model_core INTERFACE cxx_std_20)

add_library(tinyinfer::model_core ALIAS tinyinfer_model_core)

# ============================================================================
# Library: tinyinfer_backend_mock — Mock 模型后端
# ============================================================================
add_library(tinyinfer_backend_mock INTERFACE)
target_include_directories(tinyinfer_backend_mock INTERFACE "${TINYINFER_INCLUDE_DIR}")
target_link_libraries(tinyinfer_backend_mock INTERFACE tinyinfer::model_core)
target_compile_features(tinyinfer_backend_mock INTERFACE cxx_std_20)

add_library(tinyinfer::backend_mock ALIAS tinyinfer_backend_mock)

# ============================================================================
# Library: tinyinfer_tokenizer — Tokenizer 抽象
# ============================================================================
add_library(tinyinfer_tokenizer INTERFACE)
target_include_directories(tinyinfer_tokenizer INTERFACE "${TINYINFER_INCLUDE_DIR}")
target_link_libraries(tinyinfer_tokenizer INTERFACE tinyinfer::common)
target_compile_features(tinyinfer_tokenizer INTERFACE cxx_std_20)

add_library(tinyinfer::tokenizer ALIAS tinyinfer_tokenizer)

# ============================================================================
# Library: tinyinfer_application — 应用服务层
# ============================================================================
add_library(tinyinfer_application INTERFACE)
target_include_directories(tinyinfer_application INTERFACE "${TINYINFER_INCLUDE_DIR}")
target_link_libraries(tinyinfer_application INTERFACE
  tinyinfer::domain
  tinyinfer::scheduler
  tinyinfer::tokenizer
  tinyinfer::model_core
)
target_compile_features(tinyinfer_application INTERFACE cxx_std_20)

add_library(tinyinfer::application ALIAS tinyinfer_application)

# ============================================================================
# Executable: tinyinfer_cli — 命令行入口
# ============================================================================
add_executable(tinyinfer_cli
  src/main.cpp
)

target_link_libraries(tinyinfer_cli PRIVATE
  tinyinfer::common
)

# ============================================================================
# 测试
# ============================================================================
if(TINYINFER_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# ============================================================================
# 安装规则
# ============================================================================
include(GNUInstallDirs)
# TODO: configure_package_config_file when installation support is needed

install(TARGETS tinyinfer_cli
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

install(DIRECTORY "${TINYINFER_INCLUDE_DIR}/tinyinfer"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

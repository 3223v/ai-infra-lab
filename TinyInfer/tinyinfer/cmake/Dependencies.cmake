# ==============================================================================
# TinyInfer 依赖管理
# ==============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# GoogleTest (测试框架)
# ----------------------------------------------------------------------------
if(TINYINFER_BUILD_TESTS)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
    GIT_SHALLOW    TRUE
  )

  # 防止 GoogleTest 覆盖父项目的编译选项
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(googletest)
endif()

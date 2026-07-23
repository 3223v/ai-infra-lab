# =============================================================================
# TinyInfer — 测试 CMakeLists
# =============================================================================

# ---------------------------------------------------------------------------
# Common 单元测试
# ---------------------------------------------------------------------------
add_executable(tinyinfer_test_common
  unit/common/common_test.cpp
  unit/domain/finish_reason_test.cpp
)

target_link_libraries(tinyinfer_test_common PRIVATE
  tinyinfer::common
  tinyinfer::domain
  GTest::gtest
  GTest::gtest_main
)

target_compile_features(tinyinfer_test_common PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(tinyinfer_test_common
  TEST_PREFIX "unit/"
)

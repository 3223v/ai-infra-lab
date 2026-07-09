// ============================================================================
// test_tensor.cpp — DataType / Shape 单元测试
// ============================================================================
#include <catch2/catch_test_macros.hpp>
#include "tinyinfer/tensor.hpp"

using namespace tinyinfer;

// ---- DataType ----

// 验证每种类型的字节数正确
// 这很重要：后续读写 weights.bin 和分配 GPU 显存都依赖这个
TEST_CASE("dtype_size returns correct bytes", "[tensor][dtype]") {
    CHECK(dtype_size(DataType::kFloat32) == 4);
    CHECK(dtype_size(DataType::kFloat16) == 2);
    CHECK(dtype_size(DataType::kInt32)   == 4);
}

// ---- Shape ----

// Shape{} 空构造 → 标量形状
TEST_CASE("default Shape has rank 0 and 0 elements", "[tensor][shape]") {
    Shape s;
    CHECK(s.rank() == 0);
    CHECK(s.num_elements() == 0);
}

// Shape{2,3,4} → rank=3, elements=24
TEST_CASE("Shape{2,3,4} has rank 3 and 24 elements", "[tensor][shape]") {
    Shape s{2, 3, 4};
    CHECK(s.rank() == 3);
    CHECK(s.num_elements() == 24);
}

// Shape{1} → rank=1, elements=1（单元素向量）
TEST_CASE("Shape{1} single-dimension", "[tensor][shape]") {
    Shape s{1};
    CHECK(s.rank() == 1);
    CHECK(s.num_elements() == 1);
}

// 两个相同 Shape 应该相等
TEST_CASE("identical Shapes are equal", "[tensor][shape]") {
    Shape a{2, 3};
    Shape b{2, 3};
    CHECK(a == b);
}

// 不同 Shape 不相等
TEST_CASE("different Shapes are not equal", "[tensor][shape]") {
    Shape a{2, 3};
    Shape b{3, 2};
    CHECK_FALSE(a == b);
}

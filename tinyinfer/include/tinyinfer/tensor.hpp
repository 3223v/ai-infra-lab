#pragma once

#include <cstdint>
#include <initializer_list>
#include <vector>

namespace tinyinfer {

// ---- DataType --------------------------------------------------------------
// 推理中涉及的数据类型
enum class DataType {
    kFloat32,  // fp32，CPU 参考路径使用
    kFloat16,  // fp16，模型存储和 GPU 推理使用
    kInt32,    // int32，token ids 使用
};

// 返回 DataType 对应的字节数
size_t dtype_size(DataType dtype);

// ---- Shape -----------------------------------------------------------------
// 张量形状，对标 PyTorch 的 torch.Size
// 例如 Shape{2, 3, 4} 表示 2×3×4 的张量
struct Shape {
    std::vector<int64_t> dims;

    // 默认构造：空形状（标量）
    Shape() = default;

    // 从 initializer_list 构造，方便写 Shape{2, 3, 4}
    Shape(std::initializer_list<int64_t> dims);

    // 总元素数 = dims[0] * dims[1] * ...，空形状返回 0
    int64_t num_elements() const;

    // 维度数 = dims.size()
    size_t rank() const;

    // 两个形状逐维比较
    bool operator==(const Shape& other) const;
};

} // namespace tinyinfer

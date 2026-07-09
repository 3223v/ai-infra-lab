#include "tinyinfer/tensor.hpp"

namespace tinyinfer {

// ---- DataType --------------------------------------------------------------
size_t dtype_size(DataType dtype) {
    switch (dtype) {
        case DataType::kFloat32: return 4;
        case DataType::kFloat16: return 2;
        case DataType::kInt32:   return 4;
    }
    return 0;  // 理论上不会到这里
}

// ---- Shape -----------------------------------------------------------------
Shape::Shape(std::initializer_list<int64_t> dims)
    : dims(dims) {}

int64_t Shape::num_elements() const {
    if (dims.empty()) return 0;
    int64_t total = 1;
    for (auto d : dims) {
        total *= d;
    }
    return total;
}

size_t Shape::rank() const {
    return dims.size();
}

bool Shape::operator==(const Shape& other) const {
    return dims == other.dims;
}

} // namespace tinyinfer

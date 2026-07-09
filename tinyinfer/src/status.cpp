#include "tinyinfer/status.hpp"

namespace tinyinfer {

// 静态工厂方法不属于任何实例，所以没有 this
// 直接调私有构造函数返回新对象
Status Status::Ok() {
    return Status(true, "");
}

Status Status::Error(const std::string& msg) {
    return Status(false, msg);
}

// ok() 和 message() 是 const 方法，不修改对象
// 直接返回成员变量即可
bool Status::ok() const {
    return ok_;
}

const std::string& Status::message() const {
    return message_;
}

// 私有构造函数，用初始化列表设置成员
Status::Status(bool ok, const std::string& msg)
    : ok_(ok), message_(msg) {}

} // namespace tinyinfer

#pragma once

#include <string>

namespace tinyinfer {

// ---- Status ----------------------------------------------------------------
// 轻量级错误状态，对标 absl::Status / Rust Result
// 不用异常是因为：推理 runtime 中错误是可预期的（文件不存在、显存不足等）
// 不需要 unwind 栈
//
// 用法：
//   Status s = load_weights(path);
//   if (!s.ok()) { std::cerr << s.message() << std::endl; return 1; }
class Status {
public:
    // 静态工厂：成功状态
    static Status Ok();

    // 静态工厂：失败状态，附带错误描述
    static Status Error(const std::string& msg);

    // 是否成功
    bool ok() const;

    // 错误信息（成功时为空字符串）
    const std::string& message() const;

private:
    bool ok_;
    std::string message_;

    // 构造函数私有，强制通过 Ok() / Error() 创建
    Status(bool ok, const std::string& msg);
};

} // namespace tinyinfer

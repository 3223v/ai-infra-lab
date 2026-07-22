// =============================================================================
// TinyInfer CLI — 命令行入口
// =============================================================================

#include <iostream>
#include <string_view>

namespace {

constexpr std::string_view kVersion = "0.1.0";

const char* build_type() {
#ifdef DEBUG_BUILD
  return "debug";
#else
  return "release";
#endif
}

}  // namespace

int main(int argc, char* argv[]) {
  if (argc > 1) {
    std::string_view arg{argv[1]};
    if (arg == "--version" || arg == "-v") {
      std::cout << "tinyinfer version " << kVersion << " (" << build_type() << ")\n";
      return 0;
    }
    if (arg == "--help" || arg == "-h") {
      std::cout
          << "TinyInfer — 单机多请求 LLM 推理框架\n\n"
          << "用法: tinyinfer_cli [选项]\n\n"
          << "选项:\n"
          << "  --version, -v    显示版本信息\n"
          << "  --help, -h       显示本帮助\n";
      return 0;
    }
  }

  // 默认输出
  std::cout << "tinyinfer " << kVersion << " — ready\n";
  return 0;
}

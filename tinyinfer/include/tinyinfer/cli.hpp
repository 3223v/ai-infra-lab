#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace tinyinfer {

// ---- CliArgs ---------------------------------------------------------------
// 解析后的命令行参数，每个字段对应一个 --xxx 选项
struct CliArgs {
    std::string model_path;
    std::vector<int> input_ids;
    int max_new_tokens = 32;
    std::string device = "cpu";     // "cpu" 或 "cuda"
    bool show_help = false;
};

// 解析 argc/argv，返回 CliArgs
// 遇到 --help 时设置 show_help=true
CliArgs parse_cli_args(int argc, char* argv[]);

// 打印使用说明到 stdout
void print_usage(const char* program_name);

} // namespace tinyinfer

#include "blockserve/workload_loader.hpp"

#include <fstream>
#include <cctype>
#include <sstream>
#include <optional>
#include <string>

namespace blockserve {
    // 其中包裹的函数为当前文件私有内部工具
    namespace {
        // 函数整体逻辑：去除字符串首尾空白
        std::string trim(const std::string& text) {
            std::size_t begin = 0;
            while(begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin]))) {
                ++begin;
            }
            std::size_t end = text.size();
            while(end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
                --end;
            }
            return text.substr(begin, end - begin);
        }
        // 根据给定 key 提取对应的无符号 64 位整数，适配格式示例："count": 12345
        std::optional<std::uint64_t> parse_uint_field(const std::string& line, const std::string& key){
            // 给key前后加上双引号
            const std::string quote_key = "\"" + key + "\"";
            // 查找 key 在字符串中的位置
            std::size_t key_pos = line.find(quote_key);
            if(key_pos == std::string::npos) {
                return std::nullopt; 
            }
            // 查找冒号的位置
            std::size_t colon_pos = line.find(':', key_pos + quote_key.size());
            if(colon_pos == std::string::npos) {
                return std::nullopt; 
            }
            // 冒号后第一个非空白字符的位置
            std::size_t value_pos = colon_pos + 1;
            while(value_pos < line.size() && std::isspace(static_cast<unsigned char>(line[value_pos]))) {
                ++value_pos;
            }
            // 如果冒号后没有数字，则返回空
            if(value_pos >= line.size()||!std::isdigit(static_cast<unsigned char>(line[value_pos]))) {
                return std::nullopt;
            }

            std::uint64_t value = 0;
            // 循环读取数字字符并构建整数值
            while(value_pos < line.size() && std::isdigit(static_cast<unsigned char>(line[value_pos]))) {
                value = value * 10 + static_cast<std::uint64_t>(line[value_pos] - '0');
                ++value_pos;
            }
            return value;
        }
        // 错误信息格式化函数，返回格式为 "line <line_number>: <message>"
        std::string make_line_error(std::uint64_t line_number, const std::string& message) {
            std::ostringstream oss;
            oss << "line " << line_number << ": " << message;
            return oss.str();
        }
    }
    // 根据errors是否为空判断加载结果是否成功
    bool WorkloadLoadResult::ok() const {
       return errors.empty();
    }
    // 读取jsonl文件并解析为WorkloadLoadResult对象
    WorkloadLoadResult load_workload_jsonl(const std::string& path){
        WorkloadLoadResult result;

        std::ifstream file(path);
        if(!file.is_open()) {
            result.errors.push_back("Failed to open file: " + path);
            return result;
        }

        std::string line;
        std::uint64_t line_number = 0;
        RequestId next_request_id = 1;

        while(std::getline(file, line)) {
            ++line_number;
            const std::string& stripped = trim(line);
            // 跳过空行
            if(stripped.empty()) {
                continue;
            }
            if(!stripped.empty() && stripped[0] == '#') {
                continue;
            }

            const auto id = parse_uint_field(stripped, "id");
            const auto arrival_time = parse_uint_field(stripped, "arrival_time");
            const auto prompt_len = parse_uint_field(stripped, "prompt_len");
            const auto max_new_tokens = parse_uint_field(stripped, "max_new_tokens");

            if(!arrival_time.has_value()) {
                result.errors.push_back(make_line_error(line_number, "Missing or invalid 'arrival_time' field"));
                continue;
            }
            if(!prompt_len.has_value()) {
                result.errors.push_back(make_line_error(line_number, "Missing or invalid 'prompt_len' field"));
                continue;
            }
            if(!max_new_tokens.has_value()) {
                result.errors.push_back(make_line_error(line_number, "Missing or invalid 'max_new_tokens' field"));
                continue;
            }
            if(*prompt_len == 0) {
                result.errors.push_back(make_line_error(line_number, "'prompt_len' must be greater than 0"));
                continue;
            }
            if(*max_new_tokens == 0) {
                result.errors.push_back(make_line_error(line_number, "'max_new_tokens' must be greater than 0"));
                continue;
            }

            RequestId request_id = id.value_or(next_request_id++);

            if(request_id >= next_request_id) {
                next_request_id = request_id + 1;
            }else{
                ++next_request_id;
            }
            Request request{
                .id = request_id,
                .status = RequestStatus::WAITING,
                .arrival_time = *arrival_time,
                .prompt_len = static_cast<std::uint32_t>(*prompt_len),
                .max_new_tokens = static_cast<std::uint32_t>(*max_new_tokens),
                .generated_tokens = 0
            };
            result.requests.push_back(request);
        }
        return result;
    }
}
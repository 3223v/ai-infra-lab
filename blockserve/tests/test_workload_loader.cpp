#include <catch2/catch_test_macros.hpp>

#include "blockserve/workload_loader.hpp"
#include <fstream>
#include <cstdio>   // std::remove

using namespace blockserve;

// ============================================================
// 正常 workload 加载
// ============================================================

// 场景：加载 sample_workload.jsonl（5 条合法记录）
// 验证 ok() 为 true 且 requests 数量正确
TEST_CASE("加载 sample_workload — 成功解析 5 条请求", "[loader][happy]") {
    auto result = load_workload_jsonl("blockserve/configs/sample_workload.jsonl");

    // ok() 是约定的错误检查入口：没有 error 才算成功
    REQUIRE(result.ok());
    CHECK(result.requests.size() == 5);
    CHECK(result.errors.empty());
}

// 场景：验证解析出的第一条请求字段值
// 不只看数量，还要看内容对不对
TEST_CASE("sample_workload 第一条记录字段正确", "[loader][happy]") {
    auto result = load_workload_jsonl("blockserve/configs/sample_workload.jsonl");
    REQUIRE(result.ok());

    const auto& first = result.requests.at(0);
    CHECK(first.arrival_time   == 0);
    CHECK(first.prompt_len     == 64);
    CHECK(first.max_new_tokens == 20);
    CHECK(first.status         == RequestStatus::WAITING);
    CHECK(first.generated_tokens == 0);
}

// ============================================================
// 自动 ID 分配
// ============================================================

// 场景：JSON 中没有 "id" 字段时，loader 自动从 1 开始分配
// 这是 workload_loader.cpp 的 next_request_id 逻辑
TEST_CASE("无 id 字段时自动从 1 递增", "[loader][auto_id]") {
    auto result = load_workload_jsonl("blockserve/configs/sample_workload.jsonl");
    REQUIRE(result.ok());

    // 5 条记录都没有 id 字段，应分配 1, 2, 3, 4, 5
    for (std::size_t i = 0; i < result.requests.size(); ++i) {
        CHECK(result.requests[i].id == static_cast<RequestId>(i + 1));
    }
}

// ============================================================
// 坏 workload — 错误处理
// ============================================================

// 场景：加载 bad_workload.jsonl
// 内容：第 2 行 prompt_len=0（非法），第 3 行缺少 max_new_tokens
// 预期：第 1 行正常加载，第 2、3 行报错
TEST_CASE("加载 bad_workload — 返回 errors", "[loader][error]") {
    auto result = load_workload_jsonl("blockserve/configs/bad_workload.jsonl");

    // ok() 应为 false（有 error）
    CHECK_FALSE(result.ok());
    // 第 1 行没问题，应加载成功
    CHECK(result.requests.size() == 1);
    // 第 2、3 行各产生一个 error
    CHECK(result.errors.size() == 2);
}

// 场景：prompt_len=0 被拒绝
// 原理：prompt_len 为 0 意味着空请求，没有任何 token 要处理，应该是非法输入
TEST_CASE("prompt_len 为 0 被拒绝", "[loader][error]") {
    auto result = load_workload_jsonl("blockserve/configs/bad_workload.jsonl");
    CHECK_FALSE(result.ok());

    // 第二条 error 应该提到 prompt_len
    bool found = false;
    for (const auto& err : result.errors) {
        if (err.find("prompt_len") != std::string::npos) {
            found = true;
            break;
        }
    }
    CHECK(found);
}

// 场景：缺少 max_new_tokens 被拒绝
// 原理：没有 max_new_tokens 意味着不知道生成多少个 token，无法调度
TEST_CASE("缺少 max_new_tokens 被拒绝", "[loader][error]") {
    auto result = load_workload_jsonl("blockserve/configs/bad_workload.jsonl");
    CHECK_FALSE(result.ok());

    // 第三条 error 应该提到 max_new_tokens
    bool found = false;
    for (const auto& err : result.errors) {
        if (err.find("max_new_tokens") != std::string::npos) {
            found = true;
            break;
        }
    }
    CHECK(found);
}

// 场景：文件不存在
// 原理：std::ifstream 打不开文件时，loader 应立即报错，不 crash
TEST_CASE("文件不存在 — 返回错误", "[loader][error]") {
    auto result = load_workload_jsonl("blockserve/configs/does_not_exist.jsonl");

    CHECK_FALSE(result.ok());
    CHECK(result.requests.empty());
    // 至少有一条 error 提到文件打不开
    REQUIRE(result.errors.size() >= 1);
    CHECK(result.errors[0].find("Failed to open") != std::string::npos);
}

// ============================================================
// 手动写入临时 workload — 验证跳过空行和注释
// ============================================================

// 场景：JSONL 中包含空行和 # 注释行，应被正确跳过
// 原理：loader 中 trimmed.empty() 和 stripped[0]=='#' 的判断逻辑
TEST_CASE("空行和注释行被跳过", "[loader][edge]") {
    // 写入临时文件
    const char* tmp_path = "/tmp/test_skip_lines.jsonl";
    {
        std::ofstream f(tmp_path);
        f << "\n";                                              // 空行
        f << "# 这是一行注释\n";                                 // 注释行
        f << "{\"arrival_time\":1,\"prompt_len\":10,\"max_new_tokens\":5}\n";
        f << "   \n";                                           // 只含空格的行
        f << "{\"arrival_time\":2,\"prompt_len\":20,\"max_new_tokens\":10}\n";
        f << "# 最后一行注释\n";
    }

    auto result = load_workload_jsonl(tmp_path);
    std::remove(tmp_path);  // 清理临时文件

    // 应该只解析出 2 条有效记录
    REQUIRE(result.ok());
    CHECK(result.requests.size() == 2);
    CHECK(result.requests[0].arrival_time == 1);
    CHECK(result.requests[1].arrival_time == 2);
}

// ============================================================
// 显式指定 ID
// ============================================================

// 场景：JSON 中显式给出 "id": 100，应保留该 ID 而不自动分配
// 原理：当 parse_uint_field("id") 有值时，直接使用该值
TEST_CASE("显式指定 id 字段 — 保留给定值", "[loader][explicit_id]") {
    const char* tmp_path = "/tmp/test_explicit_id.jsonl";
    {
        std::ofstream f(tmp_path);
        f << "{\"id\":100,\"arrival_time\":1,\"prompt_len\":10,\"max_new_tokens\":5}\n";
        f << "{\"id\":200,\"arrival_time\":2,\"prompt_len\":20,\"max_new_tokens\":10}\n";
    }

    auto result = load_workload_jsonl(tmp_path);
    std::remove(tmp_path);

    REQUIRE(result.ok());
    CHECK(result.requests.size() == 2);
    CHECK(result.requests[0].id == 100);
    CHECK(result.requests[1].id == 200);
}

// 场景：混合显式 ID 和自动 ID
// 先给 id=5，下一条不给，应自动分配 6（max(5, 当前auto)+1）
TEST_CASE("显式 ID 和自动 ID 混合 — 自动递增不冲突", "[loader][explicit_id]") {
    const char* tmp_path = "/tmp/test_mixed_id.jsonl";
    {
        std::ofstream f(tmp_path);
        f << "{\"id\":5,\"arrival_time\":1,\"prompt_len\":10,\"max_new_tokens\":5}\n";
        f << "{\"arrival_time\":2,\"prompt_len\":20,\"max_new_tokens\":10}\n";  // 无 id → 应分配 6
        f << "{\"arrival_time\":3,\"prompt_len\":30,\"max_new_tokens\":15}\n";  // 无 id → 应分配 7
    }

    auto result = load_workload_jsonl(tmp_path);
    std::remove(tmp_path);

    REQUIRE(result.ok());
    CHECK(result.requests.size() == 3);
    CHECK(result.requests[0].id == 5);
    CHECK(result.requests[1].id == 6);
    CHECK(result.requests[2].id == 7);
}

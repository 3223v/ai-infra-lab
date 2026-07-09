#include <catch2/catch_test_macros.hpp>

#include "blockserve/request_queue.hpp"

using namespace blockserve;

// 辅助函数：快速造一个 Request，只关心 id，其余字段给默认值
// 这样测试代码不会被无关字段干扰，一眼能看出"在测哪个雷quest"
namespace {
    Request make_req(RequestId id) {
        return Request{id, RequestStatus::WAITING, 0, 0, 0, 0};
    }
}

// ============================================================
// 基础 push / pop — 验证队列的 FIFO 语义
// ============================================================

// 场景：push 一个请求后立刻 pop，应该拿到同一个请求
// 这是队列最核心的语义——先入先出
TEST_CASE("push 一个再 pop 一个 — 字段完全一致", "[queue][fifo]") {
    RequestQueue q;
    Request original{42, RequestStatus::WAITING, 5, 128, 64, 0};

    q.push(original);
    auto result = q.pop();

    // REQUIRE: 如果失败说明队列坏了，后面断言也没意义
    REQUIRE(result.has_value());
    // 逐一比对关键字段，确保不是同一个"壳"
    CHECK(result->id             == 42);
    CHECK(result->arrival_time   == 5);
    CHECK(result->prompt_len     == 128);
    CHECK(result->max_new_tokens == 64);
    CHECK(result->generated_tokens == 0);
}

// 场景：push 三个请求，pop 三个，验证顺序保持
// 如果队列实现错了（比如用了 stack），这里会挂
TEST_CASE("push 三个 pop 三个 — 顺序保持", "[queue][fifo]") {
    RequestQueue q;
    q.push(make_req(1));
    q.push(make_req(2));
    q.push(make_req(3));

    CHECK(q.pop()->id == 1);
    CHECK(q.pop()->id == 2);
    CHECK(q.pop()->id == 3);
}

// ============================================================
// 空队列行为 — 防御性编程的关键场景
// ============================================================

// 场景：空队列调用 pop() 必须返回 nullopt，不能崩溃
// 原理：std::optional 是"可能没有值"的类型安全表达
TEST_CASE("空队列 pop 返回 nullopt", "[queue][empty]") {
    RequestQueue q;
    auto result = q.pop();
    CHECK_FALSE(result.has_value());
}

// 场景：空队列调用 front() 返回 nullptr
// front() 是"看一眼队首但不拿走"，空时不能解引用
TEST_CASE("空队列 front 返回 nullptr", "[queue][empty]") {
    RequestQueue q;
    CHECK(q.front() == nullptr);
}

// 场景：空队列 empty() 返回 true
TEST_CASE("空队列 empty 为 true", "[queue][empty]") {
    RequestQueue q;
    CHECK(q.empty() == true);
}

// ============================================================
// size 跟踪 — 验证 push/pop 对 size 的影响
// ============================================================

// 场景：每 push 一次 size+1，每 pop 一次 size-1
TEST_CASE("size 随 push/pop 正确增减", "[queue][size]") {
    RequestQueue q;
    CHECK(q.size() == 0);   // 初始为空

    q.push(make_req(1));
    CHECK(q.size() == 1);

    q.push(make_req(2));
    CHECK(q.size() == 2);

    q.pop();
    CHECK(q.size() == 1);   // pop 后减 1

    q.pop();
    CHECK(q.size() == 0);   // 全 pop 完回到 0
}

// ============================================================
// front() — 看一眼但不拿走
// ============================================================

// 场景：front() 返回指针指向队首，但不会移除元素
// 连续两次 front() 应该返回同一个元素
TEST_CASE("front 只读不删 — 两次 front 相同", "[queue][front]") {
    RequestQueue q;
    q.push(make_req(99));

    const Request* first  = q.front();
    const Request* second = q.front();

    REQUIRE(first != nullptr);
    CHECK(first->id == 99);
    CHECK(second == first);         // 同一个指针，说明没被删
    CHECK(q.size() == 1);           // size 也没变
}

// ============================================================
// push/pop 交替 — 验证队列在混合操作下不乱
// ============================================================

// 场景：push 两个 → pop 一个 → push 一个 → pop 两个
// 模拟实际使用中请求陆续到达、陆续被取走的场景
TEST_CASE("push/pop 交替操作 — 队列状态始终正确", "[queue][mixed]") {
    RequestQueue q;

    q.push(make_req(10));
    q.push(make_req(20));
    CHECK(q.size() == 2);

    CHECK(q.pop()->id == 10);   // 先入先出
    CHECK(q.size() == 1);

    q.push(make_req(30));       // 此时队列: [20, 30]
    CHECK(q.size() == 2);

    CHECK(q.pop()->id == 20);
    CHECK(q.pop()->id == 30);
    CHECK(q.empty());
}

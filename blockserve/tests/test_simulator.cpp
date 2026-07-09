#include <catch2/catch_test_macros.hpp>

#include "blockserve/simulator.hpp"

using namespace blockserve;

// 辅助函数：快速造 Request，只需指定 id 和 arrival_time
namespace {
    Request make_req(RequestId id, std::uint64_t arrival_time) {
        return Request{id, RequestStatus::WAITING, arrival_time, 1, 1, 0};
    }
}

// ============================================================
// 构造函数 — 排序验证
// ============================================================

// 场景：传入乱序的请求列表，构造函数应将其按 arrival_time 升序排列
// 原理：Simulator 构造函数中的 std::sort，按 arrival_time 排序
// 验证方式：构造后逐步 step，看谁先到达
TEST_CASE("构造函数按 arrival_time 排序", "[simulator][ctor]") {
    // 故意给乱序：t=5, t=0, t=10
    std::vector<Request> input;
    input.push_back(make_req(3, 5));
    input.push_back(make_req(1, 0));
    input.push_back(make_req(2, 10));

    Simulator sim(input);

    // step 推进到 t=1：只有 t=0 的请求应该到达
    sim.step();
    CHECK(sim.current_time() == 1);
    CHECK(sim.waiting_queue_size() == 1);
    CHECK_FALSE(sim.all_arrived());
}

// ============================================================
// step() — 时间推进与请求到达
// ============================================================

// 场景：单个请求 t=5，step 到 t=5 之前不应该到达
// 到达条件：it->arrival_time <= current_time_
TEST_CASE("请求在到达时间之前不会出现", "[simulator][step]") {
    std::vector<Request> input;
    input.push_back(make_req(1, 5));

    Simulator sim(input);

    // 推进到 t=4
    for (int i = 0; i < 4; ++i) {
        sim.step();
    }
    CHECK(sim.waiting_queue_size() == 0);   // 还没到 t=5
    CHECK_FALSE(sim.all_arrived());
}

// 场景：到 t=5 时请求到达
TEST_CASE("到达到达时间时请求进入 waiting_queue", "[simulator][step]") {
    std::vector<Request> input;
    input.push_back(make_req(1, 5));

    Simulator sim(input);

    for (int i = 0; i < 5; ++i) {
        sim.step();  // 最后一次推进到 t=5
    }
    CHECK(sim.waiting_queue_size() == 1);
}

// 场景：同一时刻到达多个请求
// 比如两个请求 arrival_time 都是 3，step 到 t=3 时两个一起进入
TEST_CASE("同一时刻多个请求同时到达", "[simulator][step]") {
    std::vector<Request> input;
    input.push_back(make_req(1, 3));
    input.push_back(make_req(2, 3));
    input.push_back(make_req(3, 3));

    Simulator sim(input);

    for (int i = 0; i < 3; ++i) {
        sim.step();
    }

    // t=3 时三个请求应全部到达
    CHECK(sim.waiting_queue_size() == 3);
}

// ============================================================
// all_arrived() — 判断是否所有请求都已到达
// ============================================================

// 场景：初始时 pending 有请求，all_arrived 应为 false
TEST_CASE("有 pending 请求时 all_arrived 为 false", "[simulator][all_arrived]") {
    std::vector<Request> input;
    input.push_back(make_req(1, 10));

    Simulator sim(input);
    CHECK_FALSE(sim.all_arrived());
}

// 场景：所有请求到达后 all_arrived 为 true
// 注意：此时 waiting_queue 非空（请求在等待处理），但 pending 已空
TEST_CASE("全部到达后 all_arrived 为 true — 即使 waiting_queue 非空", "[simulator][all_arrived]") {
    std::vector<Request> input;
    input.push_back(make_req(1, 0));

    Simulator sim(input);
    sim.step();  // t=1, 请求到达

    CHECK(sim.all_arrived() == true);
    CHECK(sim.waiting_queue_size() == 1);  // queue 非空，但 pending 空了
}

// ============================================================
// is_done() — 判断模拟是否完全结束
// ============================================================

// 场景：is_done 需要 pending 和 waiting_queue 都为空
// 与 all_arrived 的区别：all_arrived 只看 pending，is_done 两者都看
TEST_CASE("is_done 在 pending 和 queue 都空时为 true", "[simulator][is_done]") {
    // 传空列表 → pending 空 + queue 空 → is_done 应该为 true
    std::vector<Request> input;
    Simulator sim(input);

    CHECK(sim.is_done() == true);
    CHECK(sim.all_arrived() == true);
}

// 场景：pending 空但 queue 非空 → is_done 应为 false
// 这个场景 Day 4 的代码可能会有问题（见上一个对话的 bug 修复），测试确保不回归
TEST_CASE("pending 空但 queue 非空 — is_done 为 false", "[simulator][is_done]") {
    std::vector<Request> input;
    input.push_back(make_req(1, 0));

    Simulator sim(input);
    sim.step();  // 请求到达，pending 空，queue 非空

    CHECK(sim.all_arrived() == true);   // pending 空
    CHECK(sim.is_done() == false);      // queue 还有东西
}

// ============================================================
// run_until() — 自动推进到指定的时间或全部到达
// ============================================================

// 场景：所有请求在 max_time 之前到达，应提前停止
// run_until 的条件：current_time < max_time && !all_arrived()
TEST_CASE("run_until 在所有请求到达后提前停止", "[simulator][run_until]") {
    std::vector<Request> input;
    input.push_back(make_req(1, 3));
    input.push_back(make_req(2, 5));

    Simulator sim(input);
    sim.run_until(100);  // max_time 很大

    // 应该在 t=5 后停止（不是跑到 100）
    // 过程：t=5 时 step()→t=5, 请求2到达, all_arrived=true → 循环退出
    CHECK(sim.current_time() == 5);
    CHECK(sim.all_arrived());
}

// 场景：如果 max_time 太小，未全部到达就停止
TEST_CASE("run_until 在 max_time 到达时停止 — 即使请求未全部到达", "[simulator][run_until]") {
    std::vector<Request> input;
    input.push_back(make_req(1, 10));   // t=10 才到
    input.push_back(make_req(2, 20));   // t=20 才到

    Simulator sim(input);
    sim.run_until(5);  // 只给 5 个时间单位

    CHECK(sim.current_time() == 5);
    CHECK_FALSE(sim.all_arrived());
    // arrival_time 最小是 10，max_time=5，请求全部未到达
    CHECK(sim.waiting_queue_size() == 0);
}

// 修正上面的逻辑：arrival_time=10 > max_time=5，请求未到达
TEST_CASE("run_until 提前停止 — waiting_queue 只有已到达的请求", "[simulator][run_until]") {
    std::vector<Request> input;
    input.push_back(make_req(1, 2));    // t=2 到达
    input.push_back(make_req(2, 10));   // t=10 到达（不会到达）

    Simulator sim(input);
    sim.run_until(5);

    CHECK(sim.current_time() == 5);
    CHECK_FALSE(sim.all_arrived());
    CHECK(sim.waiting_queue_size() == 1);  // 只有 id=1 到了
}

// ============================================================
// waiting_queue_size() — 逐步验证
// ============================================================

// 场景：逐步 step，验证每步的 queue size 变化
TEST_CASE("逐步 step — waiting_queue_size 按预期增长", "[simulator][size]") {
    std::vector<Request> input;
    input.push_back(make_req(1, 1));
    input.push_back(make_req(2, 2));
    input.push_back(make_req(3, 3));

    Simulator sim(input);

    CHECK(sim.waiting_queue_size() == 0);  // 初始

    sim.step();  // t=1
    CHECK(sim.waiting_queue_size() == 1);

    sim.step();  // t=2
    CHECK(sim.waiting_queue_size() == 2);

    sim.step();  // t=3
    CHECK(sim.waiting_queue_size() == 3);
}

// ============================================================
// 空输入 — 边界情况
// ============================================================

// 场景：传入空的请求列表，Simulator 应该能正常构造且立即 is_done
TEST_CASE("空输入 — 立即 is_done", "[simulator][edge]") {
    std::vector<Request> empty;
    Simulator sim(empty);

    CHECK(sim.is_done() == true);
    CHECK(sim.all_arrived() == true);
    CHECK(sim.waiting_queue_size() == 0);
    CHECK(sim.current_time() == 0);
}

// 场景：空输入 step() 仍然可以调用，不会崩溃
// step 中 pending_requests_ 为空，for 循环直接跳过，只打印一行日志
TEST_CASE("空输入 step 不崩溃", "[simulator][edge]") {
    std::vector<Request> empty;
    Simulator sim(empty);
    sim.step();  // 不应 crash
    CHECK(sim.current_time() == 1);
}

#include <catch2/catch_test_macros.hpp>

#include "blockserve/request.hpp"
#include "blockserve/request_queue.hpp"

using namespace blockserve;

// ============================================================
// RequestStatus — to_string
// ============================================================
TEST_CASE("to_string returns correct string for each status", "[request][to_string]") {
    CHECK(to_string(RequestStatus::WAITING)   == "Waiting");
    CHECK(to_string(RequestStatus::PREFILLING) == "Prefilling");
    CHECK(to_string(RequestStatus::DECODING)  == "Decoding");
    CHECK(to_string(RequestStatus::FINISHED)  == "Finished");
    CHECK(to_string(RequestStatus::CANCELED)  == "Canceled");
    CHECK(to_string(RequestStatus::REJECTED)  == "Rejected");
    CHECK(to_string(RequestStatus::TIMED_OUT) == "TimedOut");
}

// ============================================================
// RequestStatus — is_terminal_status
// ============================================================
TEST_CASE("terminal states return true", "[request][terminal]") {
    CHECK(is_terminal_status(RequestStatus::FINISHED)  == true);
    CHECK(is_terminal_status(RequestStatus::CANCELED)  == true);
    CHECK(is_terminal_status(RequestStatus::REJECTED)  == true);
    CHECK(is_terminal_status(RequestStatus::TIMED_OUT) == true);
}

TEST_CASE("active states return false", "[request][terminal]") {
    CHECK(is_terminal_status(RequestStatus::WAITING)    == false);
    CHECK(is_terminal_status(RequestStatus::PREFILLING) == false);
    CHECK(is_terminal_status(RequestStatus::DECODING)   == false);
}

// ============================================================
// RequestQueue — push / pop / size
// ============================================================
TEST_CASE("push then pop returns the same request", "[queue][push_pop]") {
    RequestQueue queue;
    Request req{42, RequestStatus::WAITING, 5, 128, 32, 0};

    queue.push(req);
    auto result = queue.pop();

    REQUIRE(result.has_value());
    CHECK(result->id             == 42);
    CHECK(result->arrival_time   == 5);
    CHECK(result->prompt_len     == 128);
    CHECK(result->max_new_tokens == 32);
}

TEST_CASE("pop from empty queue returns nullopt", "[queue][empty]") {
    RequestQueue queue;
    auto result = queue.pop();
    CHECK_FALSE(result.has_value());
}

TEST_CASE("queue size increases with push", "[queue][size]") {
    RequestQueue queue;
    CHECK(queue.size() == 0);

    Request req{0, RequestStatus::WAITING, 0, 0, 0, 0};
    queue.push(req);
    CHECK(queue.size() == 1);

    queue.push(req);
    queue.push(req);
    CHECK(queue.size() == 3);
}

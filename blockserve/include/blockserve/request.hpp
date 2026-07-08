#pragma once 
#include <cstdint>
#include <string>

namespace blockserve {
    using RequestId = uint64_t;
    enum class RequestStatus {
        WAITING,
        PREFILLING,
        DECODING,
        FINISHED,
        CANCELED,
        REJECTED,
        TIMED_OUT
    };
    std::string to_string(RequestStatus status);
    bool is_terminal_status(RequestStatus status);
    struct Request {
        RequestId id;
        RequestStatus status;
        std::uint64_t arrival_time;
        std::uint32_t prompt_len;
        std::uint32_t max_new_tokens;
        std::uint32_t generated_tokens;
    };
} // namespace blockserve
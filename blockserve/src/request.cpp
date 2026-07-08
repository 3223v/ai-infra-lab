#include "blockserve/request.hpp"
namespace blockserve {
    std::string to_string(RequestStatus status) {
        switch (status) {
            case RequestStatus::WAITING: return "Waiting";
            case RequestStatus::PREFILLING: return "Prefilling";
            case RequestStatus::DECODING: return "Decoding";
            case RequestStatus::FINISHED: return "Finished";
            case RequestStatus::CANCELED: return "Canceled";
            case RequestStatus::REJECTED: return "Rejected";
            case RequestStatus::TIMED_OUT: return "TimedOut";
            default: return "Unknown";
        }
    }
    bool is_terminal_status(RequestStatus status) {
        return status == RequestStatus::FINISHED ||
               status == RequestStatus::CANCELED ||
               status == RequestStatus::REJECTED ||
               status == RequestStatus::TIMED_OUT;
    }
} // namespace blockserve
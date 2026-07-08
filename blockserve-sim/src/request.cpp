#include "blockserve/request.hpp"
namespace blockserve {
    std::string to_string(RequestStatus status) {
        switch (status) {
            case RequestStatus::Waiting: return "Waiting";
            case RequestStatus::Prefilling: return "Prefilling";
            case RequestStatus::Decoding: return "Decoding";
            case RequestStatus::Finished: return "Finished";
            case RequestStatus::Canceled: return "Canceled";
            case RequestStatus::Rejected: return "Rejected";
            case RequestStatus::TimedOut: return "TimedOut";
            default: return "Unknown";
        }
    }
} // namespace blockserve
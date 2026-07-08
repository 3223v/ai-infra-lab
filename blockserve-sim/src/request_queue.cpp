#include "blockserve/request_queue.hpp"

namespace blockserve{
    void RequestQueue::push(const Request& request) {
        requests_.push_back(request);
 
    }
    std::optional<Request> RequestQueue::pop() {
        if (requests_.empty()) {
            return std::nullopt;
        }
        Request request = requests_.front();
        requests_.pop_front();
        return request;
    }
    const Request* RequestQueue::front() const {
        if (requests_.empty()) {
            return nullptr;
        }
        return &requests_.front();
    }
    bool RequestQueue::empty() const {
        return requests_.empty();
    }
    std::size_t RequestQueue::size() const {
        return requests_.size();
    }
}
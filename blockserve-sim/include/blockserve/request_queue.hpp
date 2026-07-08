#pragma once

#include "blockserve/request.hpp"

#include <cstdint>
#include <deque>
#include <optional>

namespace blockserve {
    class RequestQueue {
        public:
            void push(const Request& request);

            std::optional<Request> pop();

            const Request* front() const;

            bool empty() const;

            std::size_t size() const;

        private:
            std::deque<Request> requests_;
    };
}
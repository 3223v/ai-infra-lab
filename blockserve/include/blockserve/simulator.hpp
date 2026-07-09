#pragma once
#include "blockserve/request.hpp"
#include "blockserve/request_queue.hpp"
#include "blockserve/log.hpp"
#include <cstdint>



namespace blockserve {
    class Simulator {
        private:
            std::uint64_t current_time_;
            std::vector<Request> pending_requests_;
            RequestQueue waiting_queue_;
        public:
            Simulator(std::vector<Request> requests);
            std::uint64_t current_time();
            void step();
            void run_until(std::uint64_t max_time);
            bool is_done();
            std::size_t waiting_queue_size();
            bool all_arrived();
    };
} // namespace blockserve
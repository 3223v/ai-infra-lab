#include "blockserve/request.hpp"
#include "blockserve/request_queue.hpp"

#include <iostream>
#include <vector>

namespace {
    void print_request(const blockserve::Request& request) {
        std::cout << "Request ID: " << request.id << std::endl;
        std::cout << "Request Status: " << blockserve::to_string(request.status) << std::endl;
        std::cout << "Arrival Time: " << request.arrival_time << std::endl;
        std::cout << "Prompt Length: " << request.prompt_len << std::endl;
        std::cout << "Max New Tokens: " << request.max_new_tokens << std::endl;
        std::cout << "Generated Tokens: " << request.generated_tokens << std::endl;
    }
}
int main(){

    std::cout << "Blockserve-Sim started\n" << std::endl;

    std::vector<blockserve::Request> requests = {
        {1, blockserve::RequestStatus::WAITING, 0, 64, 20, 0},
        {2, blockserve::RequestStatus::PREFILLING, 1, 128, 40, 10},
        {3, blockserve::RequestStatus::DECODING, 2, 256, 80, 20},
        {4, blockserve::RequestStatus::FINISHED, 3, 512, 160, 40},
        {5, blockserve::RequestStatus::CANCELED, 4, 1024, 320, 80},
        {6, blockserve::RequestStatus::REJECTED, 5, 2048, 640, 160},
        {7, blockserve::RequestStatus::TIMED_OUT, 6, 4096, 1280, 320}
    };

    blockserve::RequestQueue queue;

    for (const auto& request : requests) {
        queue.push(request);
    }

    std::cout << "Initial queue size: " << queue.size() << "\n" << std::endl;

    while(!queue.empty()) {
        auto request = queue.pop();

        if(!request.has_value()) {
            std::cerr << "Error: Queue is empty but expected a request." << std::endl;
            break;
        }
        print_request(*request);
        std::cout << "Is Terminal: "
                  << (blockserve::is_terminal_status(request->status) ? "true" : "false")
                  << "\n\n";
    }
    std::cout << "Final queue size: " << queue.size() << std::endl;
    return 0;
}
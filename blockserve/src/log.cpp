#include "blockserve/log.hpp"

namespace blockserve{
    void log::print_request(const blockserve::Request& request) {
        std::cout << "Request ID: " << request.id << std::endl;
        std::cout << "Request Status: " << blockserve::to_string(request.status) << std::endl;
        std::cout << "Arrival Time: " << request.arrival_time << std::endl;
        std::cout << "Prompt Length: " << request.prompt_len << std::endl;
        std::cout << "Max New Tokens: " << request.max_new_tokens << std::endl;
        std::cout << "Generated Tokens: " << request.generated_tokens << std::endl;
    }
}
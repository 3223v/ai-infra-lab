#include "blockserve/request.hpp"
#include <iostream>

int main(){
    blockserve::Request request{   
        .id = 1,
        .status = blockserve::RequestStatus::Waiting,
        .arrival_time = 0,
        .prompt_len = 64,
        .max_new_tokens = 20,
        .generated_tokens = 0
    };
    std::cout << "Blockserve-Sim started\n" << std::endl;
    std::cout << "Request ID: " << request.id << std::endl;
    std::cout << "Request Status: " << blockserve::to_string(request.status) << std::endl;
    std::cout << "Arrival Time: " << request.arrival_time << std::endl;
    std::cout << "Prompt Length: " << request.prompt_len << std::endl;
    std::cout << "Max New Tokens: " << request.max_new_tokens << std::endl;
    std::cout << "Generated Tokens: " << request.generated_tokens << std::endl;
    return 0;
}
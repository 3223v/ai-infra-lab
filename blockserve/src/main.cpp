#include "blockserve/request.hpp"
#include "blockserve/request_queue.hpp"
#include "blockserve/workload_loader.hpp"

#include <iostream>
#include <vector>
#include <cstdlib>

namespace {
    void print_request(const blockserve::Request& request) {
        std::cout << "Request ID: " << request.id << std::endl;
        std::cout << "Request Status: " << blockserve::to_string(request.status) << std::endl;
        std::cout << "Arrival Time: " << request.arrival_time << std::endl;
        std::cout << "Prompt Length: " << request.prompt_len << std::endl;
        std::cout << "Max New Tokens: " << request.max_new_tokens << std::endl;
        std::cout << "Generated Tokens: " << request.generated_tokens << std::endl;
    }
}// namespace
int main(int argc, char* argv[]){

    std::cout << "Blockserve-Sim started\n" << std::endl;
    
    std::string workload_path;
    if(argc >= 2){
        workload_path = argv[1];
    }else{
        workload_path = "blockserve/configs/sample_workload.jsonl";
    }

    std::cout << "workload file: " << workload_path << std::endl;

    const blockserve::WorkloadLoadResult load_result = 
        blockserve::load_workload_jsonl(workload_path);

    if (!load_result.ok()) {
        std::cerr << "Error loading workload:" << std::endl;
        for (const auto& error : load_result.errors) {
            std::cerr << " - " << error << std::endl;
        }
        return 1;
    }

    blockserve::RequestQueue queue;

    for(const auto& request : load_result.requests) {
        queue.push(request);
    }
    std::cout << "Loaded requests: " << load_result.requests.size() << "\n" << std::endl;
    std::cout << "Initial queue size: " << queue.size() << "\n" << std::endl;

    while(!queue.empty()) {
        auto request = queue.pop();

        if(!request.has_value()) {
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
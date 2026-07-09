#include "blockserve/request.hpp"
#include "blockserve/request_queue.hpp"
#include "blockserve/workload_loader.hpp"
#include "blockserve/simulator.hpp"

#include <iostream>
#include <vector>
#include <cstdlib>

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

    blockserve::Simulator simulator(load_result.requests);
    simulator.run_until(100); // Run for 100 time units
    return 0;
}
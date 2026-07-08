#pragma once 

#include "blockserve/request.hpp"

#include <string>
#include <vector>

namespace blockserve {
    struct WorkloadLoadResult {
        std::vector<Request> requests;
        std::vector<std::string> errors;

        bool ok() const ;
    };

    WorkloadLoadResult load_workload_jsonl(const std::string& path);
} // namespace blockserve
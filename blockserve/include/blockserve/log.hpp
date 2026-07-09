#pragma once
#include<iostream>
#include "blockserve/request.hpp"

namespace blockserve{
    class log{
        public:
            static void print_request(const blockserve::Request& request);
    };
}
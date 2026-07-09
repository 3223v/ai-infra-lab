#include "blockserve/simulator.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
namespace blockserve {
    Simulator::Simulator(std::vector<Request> requests){
        current_time_ = 0;
        std::sort(requests.begin(), requests.end(), [](const Request& a, const Request& b) {
            return a.arrival_time < b.arrival_time;
        });
        pending_requests_ = requests;
    }
    void Simulator::run_until(std::uint64_t max_time){
        while(current_time_ < max_time && !all_arrived()) {
            step();
        }
    }
    bool Simulator::is_done(){
        if(pending_requests_.empty() && waiting_queue_.empty()) {
            return true;
        }
        return false;
    }
    bool Simulator::all_arrived(){
        if(pending_requests_.empty()) {
            return true;
        }
        return false;
    }
    void Simulator::step(){
        current_time_++;
        for(auto it = pending_requests_.begin(); it != pending_requests_.end();) {
            if(it->arrival_time <= current_time_) {
                waiting_queue_.push(*it);
                blockserve::log::print_request(*it);
                it = pending_requests_.erase(it);
            } else {
                ++it;
            }
        }
        std::cout << "Waiting queue size: " << waiting_queue_.size() << std::endl;
    }
    std::size_t Simulator::waiting_queue_size(){
        return waiting_queue_.size();
    }
    std::uint64_t Simulator::current_time(){
        return current_time_;
    }
}
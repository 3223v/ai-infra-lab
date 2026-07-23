// 采样参数

#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace tinyinfer {
    class SamplingParams{
        public:
            explicit SamplingParams(
                int32_t max_new_tokens = 512,
                float temperature = 1.0f,
                int32_t top_k = 40,
                float top_p = 0.95f,
                int64_t seed = -1,
                float repetition_penalty = 1.0f)
                    :max_new_tokens_(max_new_tokens),
                    temperature_(temperature),
                    top_k_(top_k),
                    top_p_(top_p),
                    seed_(seed),
                    repetition_penalty_(repetition_penalty) {}
            [[nodiscard]] int32_t max_new_tokens() const noexcept { return max_new_tokens_; }
            [[nodiscard]] float temperature() const noexcept { return temperature_; }
            [[nodiscard]] int32_t top_k() const noexcept{ return top_k_; }
            [[nodiscard]] float top_p() const noexcept{ return top_p_; }
            [[nodiscard]] int64_t seed() const noexcept{ return seed_; }
            [[nodiscard]] bool include_stop_string() const noexcept{ return include_stop_string_; }
            [[nodiscard]] float repetition_penalty() const noexcept { return repetition_penalty_; }
            
            [[nodiscard]] const std::vector<int32_t>& stop_token_ids() const noexcept { return stop_token_ids_; }
            [[nodiscard]] const std::vector<std::string>& stop_strings() const noexcept { return stop_strings_; }
            
            [[nodiscard]] bool valid() const noexcept {
                if (max_new_tokens_ <= 0) return false;
                if (temperature_ < 0.0f) return false;
                if (top_k_ < 0) return false;
                if (top_p_ < 0.0f || top_p_ > 1.0f) return false;
                if (repetition_penalty_ < 1.0f) return false;
                return true;
            }

            // 添加停止条件
            void add_stop_token(int32_t token_id) { 
                stop_token_ids_.push_back(token_id); 
            }
            void add_stop_string(std::string s) { 
                stop_strings_.push_back(std::move(s)); 
            }
            void set_include_stop_string(bool v) noexcept { 
                include_stop_string_ = v; 
            }

        private:
            // 基础长度控制
            int32_t max_new_tokens_ = 512;

            // 采样参数
            float temperature_ = 1.0f;
            int32_t top_k_ = 40;
            float top_p_ = 0.95f;

            // 随机与防重复
            int64_t seed_ = -1;
            float repetition_penalty_ = 1.0f;

            // 终止条件
            std::vector<int32_t> stop_token_ids_;
            std::vector<std::string> stop_strings_;
            bool include_stop_string_ = false;
    };

}
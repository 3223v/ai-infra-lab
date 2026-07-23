// request_id

#pragma once

namespace tinyinfer{
    class RequestId{    
        public:
            explicit RequestId(uint64_t request_id)
                : request_id_(request_id){}
            
            /// 获取原始ID值（仅用于日志、序列化、底层索引，业务逻辑优先用强类型）
            [[nodiscard]] uint64_t value() const { return id_; }

            /// 基础比较运算符
            bool operator==(const RequestId& other) const { return id_ == other.id_; }
            bool operator!=(const RequestId& other) const { return id_ != other.id_; }
            bool operator<(const RequestId& other) const { return id_ < other.id_; }
        private:
            uint64_t request_id_;
    }

} // namespace tinyinfer
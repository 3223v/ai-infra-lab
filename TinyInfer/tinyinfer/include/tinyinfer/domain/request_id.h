

#pragma once

namespace tinyinfer{
    class RequestId{    
        public:
            RequestId(uint64_t requestid)
                : request_id_(requestid){}
        private:
            uint64_t request_id_;
    }

}
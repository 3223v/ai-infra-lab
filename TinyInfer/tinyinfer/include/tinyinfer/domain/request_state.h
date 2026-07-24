// request-state

#pragma once

namespace tinyinfer{

    enum class RequestState{
        // 活跃
        kCreated;
        kTokenizing;
        kWaiting;
        kRunningPrefill;
        kRunningDecode;

        // 终止
        kFinsished;
        kCancelled;
        kTimeOut;
        kFailed;
        kRejected;

    }

    [[nodiscard]] inline constexpr bool IsTerminal(RequestState request_state){
        if (request_state == RequestState::kCreated ||
            request_state == RequestState::kTokenizing ||
            request_state == RequestState::kWaiting ||
            request_state == RequestState::kRunningPrefill ||
            request_state == RequestState::kRunningDecode
        ){
            return false;
        }
        return true;
    }


}
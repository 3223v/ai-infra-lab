// =============================================================================
// TinyInfer — 基础类型定义
// =============================================================================

#pragma once

#include <cstdint>
#include <limits>
#include <string>

namespace tinyinfer {

// ---------------------------------------------------------------------------
// Token 类型
// ---------------------------------------------------------------------------
using TokenId = int32_t;
inline constexpr TokenId kInvalidTokenId = -1;

// ---------------------------------------------------------------------------
// ID 类型
// ---------------------------------------------------------------------------
using RequestId   = uint64_t;
using SequenceId  = uint64_t;
using BatchId     = uint64_t;
using SlotId      = uint32_t;
using BlockId     = uint32_t;
using HandleId    = uint64_t;
using Generation  = uint32_t;

inline constexpr RequestId  kInvalidRequestId  = 0;
inline constexpr SequenceId kInvalidSequenceId = 0;
inline constexpr BatchId    kInvalidBatchId    = 0;
inline constexpr SlotId     kInvalidSlotId     = std::numeric_limits<SlotId>::max();
inline constexpr Generation kInvalidGeneration = 0;

// ---------------------------------------------------------------------------
// 时间类型
// ---------------------------------------------------------------------------
using TimestampNs = int64_t;

// ---------------------------------------------------------------------------
// 维度常量
// ---------------------------------------------------------------------------
inline constexpr size_t kMaxModelLayers    = 128;
inline constexpr size_t kMaxContextLen     = 131072;  // 128K
inline constexpr size_t kMaxGeneratedTokens = 65536;

}  // namespace tinyinfer

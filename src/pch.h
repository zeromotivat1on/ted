#pragma once

#include <stdint.h>
#include <assert.h>

using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

#define null nullptr

#define KB(n) (n * 1024ULL)
#define MB(n) (KB(n) * 1024ULL)
#define GB(n) (MB(n) * 1024ULL)
#define TB(n) (GB(n) * 1024ULL)

#define sign(x) ((x) > 0 ? 1 : -1)
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define clamp(x, a, b) (min((b), max((a), (x))))

#define INVALID_INDEX -1

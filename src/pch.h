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

#define BIG_ENDIAN    __BYTE_ORDER == __BIG_ENDIAN
#define LITTLE_ENDIAN __BYTE_ORDER == __LITTLE_ENDIAN

#define KB(n)   (n * 1024ull)
#define MB(n)   (KB(n) * 1024ull)
#define GB(n)   (MB(n) * 1024ull)
#define TB(n)   (GB(n) * 1024ull)

#define max(a, b) (a) > (b) ? (a) : (b)

inline u16 swap_endianness_16(const void* data)
{
    const u8* mem = (u8*)data;
    return ((u16)mem[0] << 8) | ((u16)mem[1] << 0);
}

inline u32 swap_endianness_32(const void* data)
{
    const u8* mem = (u8*)data;
    return ((u32)mem[0] << 24) | ((u32)mem[1] << 16) | ((u32)mem[2] << 8) | ((u32)mem[3] << 0);
}

inline u64 swap_endianness_64(const void* data)
{
    const u8* mem = (u8*)data;
    return ((u64)mem[0] << 56) | ((u64)mem[1] << 48) | ((u64)mem[2] << 40) | ((u64)mem[3] << 32) |
           ((u64)mem[4] << 24) | ((u64)mem[5] << 16) | ((u64)mem[6] <<  8) | ((u64)mem[7] <<  0);
}

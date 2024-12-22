#pragma once

#include <gdl/src/pch.h>
#include <gdl/src/memory.h>
#include <gdl/src/file.h>

#define BIG_ENDIAN    __BYTE_ORDER == __BIG_ENDIAN
#define LITTLE_ENDIAN __BYTE_ORDER == __LITTLE_ENDIAN

inline bool platform_little_endian()
{
    short int n = 0x1;
    char* ptr = (char*)&n;
    return (ptr[0] == 1);
}

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

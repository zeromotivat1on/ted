#pragma once

#include "gdl/src/pch.h"
#include "gdl/src/window.h"
#include "gdl/src/memory.h"
#include "gdl/src/file.h"

inline bool platform_little_endian()
{
    short int n = 0x1;
    char* ptr = (char*)&n;
    return (ptr[0] == 1);
}

inline u16 swap_endianness_16(const void* data)
{
    const u8* mem = (u8*)data;
    return (mem[0] << 8) | (mem[1] << 0);
}

inline u32 swap_endianness_32(const void* data)
{
    const u8* mem = (u8*)data;
    return (mem[0] << 24) | (mem[1] << 16) | (mem[2] << 8) | (mem[3] << 0);
}

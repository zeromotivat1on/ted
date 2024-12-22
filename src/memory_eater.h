#pragma once

inline void* eat(void** data, u32 bytes)
{
    void* ptr = *data;
    *data = (u8*)(*data) + bytes;
    return ptr;
}

inline s8 eat_s8(void** data)
{
    return *(s8*)eat(data, sizeof(s8));
}

inline u8 eat_u8(void** data)
{
    return *(u8*)eat(data, sizeof(u8));
}

inline s16 eat_s16(void** data)
{
    return *(s16*)eat(data, sizeof(s16));
}

inline u16 eat_u16(void** data)
{
    return *(u16*)eat(data, sizeof(u16));
}

inline s32 eat_s32(void** data)
{
    return *(s32*)eat(data, sizeof(s32));
}

inline u32 eat_u32(void** data)
{
    return *(u32*)eat(data, sizeof(u32));
}

inline s64 eat_s64(void** data)
{
    return *(s64*)eat(data, sizeof(s64));
}

inline u64 eat_u64(void** data)
{
    return *(u64*)eat(data, sizeof(u64));
}

inline f32 eat_f2dot14(void** data)
{
    const s16 val = eat_s16(data);
    return val / (f32)(1 << 14);
}

inline s16 eat_big_endian_s16(void** data)
{
#if LITTLE_ENDIAN
    return swap_endianness_16(eat(data, sizeof(s16)));
#else
    return eat_s16(data);
#endif
}

inline u16 eat_big_endian_u16(void** data)
{
#if LITTLE_ENDIAN
    return swap_endianness_16(eat(data, sizeof(u16)));
#else
    return eat_u16(data);
#endif
}

inline s32 eat_big_endian_s32(void** data)
{
#if LITTLE_ENDIAN
    return swap_endianness_32(eat(data, sizeof(s32)));
#else
    return eat_s32(data);
#endif
}

inline u32 eat_big_endian_u32(void** data)
{
#if LITTLE_ENDIAN
    return swap_endianness_32(eat(data, sizeof(u32)));
#else
    return eat_u32(data);
#endif
}

inline s64 eat_big_endian_s64(void** data)
{
#if LITTLE_ENDIAN
    return swap_endianness_64(eat(data, sizeof(s64)));
#else
    return eat_s64(data);
#endif
}

inline u64 eat_big_endian_u64(void** data)
{
#if LITTLE_ENDIAN
    return swap_endianness_64(eat(data, sizeof(u64)));
#else
    return eat_u64(data);
#endif
}

inline f32 eat_big_endian_f2dot14(void** data)
{
    s16 val = eat_big_endian_s16(data);    
    return val / (f32)(1 << 14);
}

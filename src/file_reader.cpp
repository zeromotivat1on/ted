#include "pch.h"
#include "file_reader.h"
#include "gdl/src/file.h"

void File_Reader::init(Arena* arena, file_handle handle)
{
    ASSERT(handle && handle != INVALID_FILE_HANDLE);
    
    file = handle;
    size = (u32)file_size(file);
    buffer = (u8*)arena->push(size);
    current = buffer;
    
    const bool file_read = read_file_sync(file, buffer, (u64)size, nullptr);
    ASSERT(file_read);
}

void* File_Reader::eat(u32 bytes)
{
    void* ptr = current;
    current += bytes;
    ASSERT(current - buffer <= size);
    return ptr;
}

s8 File_Reader::eat_s8()
{
    return *(s8*)eat(sizeof(s8));
}

u8 File_Reader::eat_u8()
{
    return *(u8*)eat(sizeof(u8));
}

s16 File_Reader::eat_s16()
{
    return *(s16*)eat(sizeof(s16));
}

u16 File_Reader::eat_u16()
{
    return *(u16*)eat(sizeof(u16));
}

s32 File_Reader::eat_s32()
{
    return *(s32*)eat(sizeof(s32));
}

u32 File_Reader::eat_u32()
{
    return *(u32*)eat(sizeof(u32));
}

s64 File_Reader::eat_s64()
{
    return *(s64*)eat(sizeof(s64));
}

u64 File_Reader::eat_u64()
{
    return *(u64*)eat(sizeof(u64));
}

void File_Reader::eat_str(char* str, u32 len)
{
    for (u32 i = 0; i < len; ++i)
    {
        str[i] = (char)eat_s8();
    }
}

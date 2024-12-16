#pragma once

struct File_Reader
{
    file_handle file;
    u8*         buffer;
    u8*         current;
    u32         size;

    void        init(Arena* arena, file_handle handle);
    void*       eat(u32 bytes);
    s8          eat_s8();
    u8          eat_u8();
    s16         eat_s16();
    u16         eat_u16();
    s32         eat_s32();
    u32         eat_u32();
    void        eat_str(char* str, u32 len);
};

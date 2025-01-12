#include "pch.h"
#include "file.h"
#include "arena.h"
#include <stdio.h>

u8* read_entire_file(Arena* arena, const char* path, s32* size_pushed)
{
    if (FILE* file = fopen(path, "rb"))
    {
        fseek(file, 0, SEEK_END);
        const s32 size = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (size_pushed)
            *size_pushed = size + 1;

        u8* file_data = push(arena, size + 1);
        const u64 read_amount = fread(file_data, size, 1, file);
        file_data[size] = '\0';

        if (read_amount)
        {
            fclose(file);
            return file_data;
        }
        
        pop(arena, size + 1);
        fclose(file);
    }

    return nullptr;
}

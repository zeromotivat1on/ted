#include "pch.h"
#include "font.h"
#include "file.h"
#include "arena.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <vendor/stb_truetype.h>

void init_font(Arena* arena, Font* font, const char* path)
{    
    s32 data_size = 0;
    u8* data = read_entire_file(arena, path, &data_size);

    font->info = push_struct(arena, stbtt_fontinfo);
    stbtt_InitFont(font->info, data, stbtt_GetFontOffsetForIndex(data, 0));
}

#pragma once

struct Arena;

struct Font
{
    struct stbtt_fontinfo* info;
};

struct Glyph_Vertices
{
    f32 data[6][4]; // quad with vertices: pos x, y, tex coord x, y
};

struct Glyph_Metrics
{
    s32 width;
    s32 height;
    s32 offset_x;
    s32 offset_y;
    s32 advance_width;
};

struct Font_Atlas
{
    u32 texture;
    s32 width;
    s32 height;
    u32 start_charcode;
    u32 end_charcode;
    s16 font_size;
};

void init_font(Arena* arena, Font* font, const char* path);
void bake_font_atlas(Arena* arena, const Font* font, Font_Atlas* atlas, u32 start_charcode, u32 end_charcode, u16 font_size);

#pragma once

// Size of batch for text glyphs to render.
// Must be the same as in shaders.
inline constexpr u16 FONT_RENDER_BATCH_SIZE = 128;

struct Arena;
struct mat4;

struct Font
{
    struct stbtt_fontinfo* info;
};

struct Font_Glyph_Metric
{
    s32 offset_x;
    s32 offset_y;
    s32 advance_width;
};

struct Font_Atlas
{
    Font_Glyph_Metric* metrics;
    u32 texture_array;
    u32 start_charcode;
    u32 end_charcode;
    s16 font_size; // also treated as size of each square glyph bitmap
    s16 line_gap;
};

struct Font_Render_Context
{
    u32 program;
    u32 vao;
    u32 vbo;
    u32* charmap;
    mat4* transforms;
};

struct Font_Render_Command
{
    const char* text;
    mat4* projection;
    f32 x, y;
    f32 r, g, b;
};

void init_font(Arena* arena, Font* font, const char* path);
void init_font_render_context(Arena* arena, Font_Render_Context* ctx);
void bake_font_atlas(Arena* arena, const Font* font, Font_Atlas* atlas, u32 start_charcode, u32 end_charcode, u16 font_size);
void render_text(Font_Render_Context* ctx, Font_Atlas* atlas, Font_Render_Command* cmd);

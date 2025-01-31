#pragma once

// Size of batch for text glyphs to render.
// Must be the same as in shaders.
inline constexpr s16 FONT_RENDER_BATCH_SIZE = 128;

struct Arena;
struct mat4;
struct Gap_Buffer;

struct Font
{
    struct stbtt_fontinfo* info;
    // Unscaled font vertical params, scale by px_h_scale from Font_Atlas.
    s32 ascent;
    s32 descent;
    s32 line_gap;
};

struct Font_Glyph_Metric
{
    s32 offset_x;
    s32 offset_y;
    s32 advance_width; // already scaled
};

struct Font_Atlas
{
    Font_Glyph_Metric* metrics;
    u32 texture_array;
    u32 start_charcode;
    u32 end_charcode;
    f32 px_h_scale;
    s32 line_height;
    s16 font_size; // size of glyph square bitmap
};

struct Font_Render_Context
{
    u32 program;
    u32 vao;
    u32 vbo;
    u32 u_charmap;
    u32 u_transforms;
    u32 u_text_color;
    u32* charmap;
    mat4* transforms;
};

void init_font(Font* font, Arena* arena, const char* path);
void init_font_render_context(Font_Render_Context* ctx, Arena* arena, s32 win_w, s32 win_h);
void bake_font_atlas(Font_Atlas* atlas, Arena* arena, const Font* font, u32 start_charcode, u32 end_charcode, s16 font_size);
void rescale_font_atlas(Font_Atlas* atlas, Arena* arena, const Font* font, s16 font_size);
void render_text(const Font_Render_Context* ctx, const Font_Atlas* atlas, const char* text, u32 size, f32 scale, f32 x, f32 y, f32 r, f32 g, f32 b);

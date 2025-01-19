#pragma once

// Size of batch for text glyphs to render.
// Must be the same as in shaders.
inline constexpr u16 FONT_RENDER_BATCH_SIZE = 128;

struct Arena;
struct mat4;
struct Gap_Buffer;

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
    u32 texture_array; // glyph bitmaps array
    u32 start_charcode;
    u32 end_charcode;
    s16 font_size; // also treated as size of each square glyph bitmap
    s16 line_gap; // vertical offset for new line
};

struct Font_Render_Context
{
    u32 program; // glyph render pipeline
    u32 vao; // glyph quad vertices data
    u32 vbo; // glyph quad vertices data
    u32* charmap; // uniform array for batch rendering
    mat4* transforms; // uniform array for batch rendering
};

void init_font(Arena* arena, Font* font, const char* path);
void init_font_render_context(Arena* arena, Font_Render_Context* ctx, s32 win_w, s32 win_h);
void bake_font_atlas(Arena* arena, const Font* font, Font_Atlas* atlas, u32 start_charcode, u32 end_charcode, u16 font_size);
void rescale_font_atlas(Arena* arena, const Font* font, Font_Atlas* atlas, u16 font_size);
// @Cleanup: get rid of copypasta.
void render_text(const Font_Render_Context* ctx, const Font_Atlas* atlas, const u32* text, u32 size, f32 scale, f32 x, f32 y, f32 r, f32 g, f32 b);
void render_text(const Font_Render_Context* ctx, const Font_Atlas* atlas, const char* text, u32 size, f32 scale, f32 x, f32 y, f32 r, f32 g, f32 b);
void render_text(const Font_Render_Context* ctx, const Font_Atlas* atlas, const Gap_Buffer* buffer, f32 scale, f32 x, f32 y, f32 r, f32 g, f32 b);
void on_framebuffer_resize(const Font_Render_Context* ctx, s32 w, s32 h);

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
    s32 offset_x; // bitmap coords
    s32 offset_y; // bitmap coords
    s32 advance_width; // already scaled
};

struct Font_Atlas
{
    Font_Glyph_Metric* metrics;
    u32 texture_array; // glyph bitmaps array
    u32 start_charcode;
    u32 end_charcode;
    f32 px_h_scale; // glyph pixel height scale
    s32 line_height;
    s16 font_size; // also treated as size of each square glyph bitmap
};

struct Font_Render_Context
{
    u32 program; // glyph render pipeline
    u32 vao; // glyph quad vertices data
    u32 vbo; // glyph quad vertices data
    u32* charmap; // uniform array for batch rendering
    mat4* transforms; // uniform array for batch rendering
};

void init_font(Font* font, Arena* arena, const char* path);
void init_font_render_context(Font_Render_Context* ctx, Arena* arena, s32 win_w, s32 win_h);
void bake_font_atlas(Font_Atlas* atlas, Arena* arena, const Font* font, u32 start_charcode, u32 end_charcode, s16 font_size);
void rescale_font_atlas(Font_Atlas* atlas, Arena* arena, const Font* font, s16 font_size);
void render_text(const Font_Render_Context* ctx, const Font_Atlas* atlas, const char* text, u32 size, f32 scale, f32 x, f32 y, f32 r, f32 g, f32 b);
void on_framebuffer_resize(const Font_Render_Context* ctx, s32 w, s32 h);

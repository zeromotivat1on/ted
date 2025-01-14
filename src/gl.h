#pragma once

#include "vector.h"
#include "matrix.h"

struct Arena;

struct GL_Uniform_Vec3
{
    const char* name;
    vec3 val;
};

struct GL_Uniform_Mat4
{
    const char* name;
    mat4 val;
};

//
// Utils
//

u32 gl_create_program(const char* vs_src, const char* fs_src);
u32 gl_load_program(Arena* arena, const char* vs_path, const char* fs_path);

//
// Debug
//



//
// Text rendering
//

struct Font;

struct GL_Glyph
{
    u32 texture;
    s32 width_px;
    s32 height_px;
    s32 offset_x_px;
    s32 offset_y_px;
    s32 advance_width_px;
};

struct GL_Glyph_Cache
{
    GL_Glyph* glyphs;
    u32 start_code;
    u32 end_code;
};

struct GL_Text_Render_Context
{
    GL_Glyph_Cache* glyph_cache;
    GL_Uniform_Mat4 u_projection;
    GL_Uniform_Mat4 u_transform;
    GL_Uniform_Vec3 u_text_color;
    u32 program;
    u32 vao;
    u32 vbo;
    s16 line_gap;
};

GL_Text_Render_Context* gl_create_text_render_context(Arena* arena, const Font* font, u32 start_code, u32 end_code, u16 font_size, s32 window_w, s32 window_h);
GL_Glyph_Cache* gl_cache_glyph_bitmaps(Arena* arena, const Font* font, u32 start_code, u32 end_code, s16 font_size);
void gl_update_glyph_bitmaps(const Font* font, GL_Glyph_Cache* cache, s16 font_size);
void gl_render_text(GL_Text_Render_Context* ctx, vec2 pos, const char* text);

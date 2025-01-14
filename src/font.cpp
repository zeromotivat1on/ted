#include "pch.h"
#include "font.h"
#include "file.h"
#include "arena.h"
#include "matrix.h"
#include "gl.h"
#include <stdio.h>
#include <glad/glad.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <vendor/stb_truetype.h>

void init_font(Arena* arena, Font* font, const char* path)
{    
    s32 data_size = 0;
    u8* data = read_entire_file(arena, path, &data_size);

    font->info = push_struct(arena, stbtt_fontinfo);
    stbtt_InitFont(font->info, data, stbtt_GetFontOffsetForIndex(data, 0));
}

void init_font_render_context(Arena* arena, Font_Render_Context* ctx, s32 win_w, s32 win_h)
{
    ctx->program = gl_load_program(arena, DIR_SHADERS "text_batch_2d.vs", DIR_SHADERS "text_batch_2d.fs");
    on_window_resize(ctx, win_w, win_h);

    ctx->charmap = push_array(arena, FONT_RENDER_BATCH_SIZE, u32);
    ctx->transforms = push_array(arena, FONT_RENDER_BATCH_SIZE, mat4);
    
    glGenVertexArrays(1, &ctx->vao);
    glGenBuffers(1, &ctx->vbo);
    
    glBindVertexArray(ctx->vao);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);

    f32 vertices[4 * 2] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
    };
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void bake_font_atlas(Arena* arena, const Font* font, Font_Atlas* atlas, u32 start_charcode, u32 end_charcode, u16 font_size)
{
    const u32 charcode_count = end_charcode - start_charcode + 1;
    atlas->metrics = push_array(arena, charcode_count, Font_Glyph_Metric);
    atlas->start_charcode = start_charcode;
    atlas->end_charcode = end_charcode;
    atlas->font_size = font_size;
    
    glGenTextures(1, &atlas->texture_array);
    rescale_font_atlas(arena, font, atlas, font_size);
}

void rescale_font_atlas(Arena* arena, const Font* font, Font_Atlas* atlas, u16 font_size)
{
    atlas->font_size = font_size;
    
    const f32 scale = stbtt_ScaleForPixelHeight(font->info, (f32)font_size);
    const u32 charcode_count = atlas->end_charcode - atlas->start_charcode + 1;
    
    s32 ascent, descent, line_gap;
    stbtt_GetFontVMetrics(font->info, &ascent, &descent, &line_gap);
    atlas->line_gap = (s16)((ascent - descent + line_gap) * scale);
    
    // stbtt rasterizes glyphs as 8bpp, so tell open gl to use 1 byte per color channel.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glBindTexture(GL_TEXTURE_2D_ARRAY, atlas->texture_array);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, atlas->font_size, atlas->font_size, charcode_count, 0, GL_RED, GL_UNSIGNED_BYTE, null);
    
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    u8* bitmap = push(arena, font_size * font_size);    
    for (u32 i = 0; i < charcode_count; ++i)
    {
        const u32 c = i + atlas->start_charcode;
        Font_Glyph_Metric* metric = atlas->metrics + i;
        
        s32 w, h, offx, offy;
        const s32 glyph_index = stbtt_FindGlyphIndex(font->info, c);
        u8* stb_bitmap = stbtt_GetGlyphBitmap(font->info, scale, scale, glyph_index, &w, &h, &offx, &offy);

        // Offset original bitmap to be at center of new one.
        const s32 x_offset = (font_size - w) / 2;
        const s32 y_offset = (font_size - h) / 2;

        metric->offset_x = offx - x_offset;
        metric->offset_y = offy - y_offset;
        
        memset(bitmap, 0, font_size * font_size);

        // @Cleanup: looks nasty, should come up with better solution.
        // Now we bake bitmap using stbtt and 'rescale' it to font size square one.
        // Maybe use stbtt_MakeGlyphBitmap at least?
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                int src_index = y * w + x;
                int dest_index = (y + y_offset) * font_size + (x + x_offset);
                if (dest_index >= 0 && dest_index < font_size * font_size)
                    bitmap[dest_index] = stb_bitmap[src_index];
            }
        }

        stbtt_FreeBitmap(stb_bitmap, null);

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, font_size, font_size, 1, GL_RED, GL_UNSIGNED_BYTE, bitmap);

        s32 advance_width = 0;
        stbtt_GetGlyphHMetrics(font->info, glyph_index, &advance_width, 0);
        metric->advance_width = (s32)(advance_width * scale);
    }

    pop(arena, font_size * font_size);
    
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    // Restore default color channel size.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);    
}

void render_text(const Font_Render_Context* ctx, const Font_Atlas* atlas, const u32* text, f32 scale, f32 x, f32 y, f32 r, f32 g, f32 b)
{
    glUseProgram(ctx->program);
    glBindVertexArray(ctx->vao);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
    glBindTexture(GL_TEXTURE_2D_ARRAY, atlas->texture_array);
    
    glActiveTexture(GL_TEXTURE0);
    glUniform3f(glGetUniformLocation(ctx->program, "u_text_color"), r, g, b);

    const u32* c = text;
    
    s32 work_idx = 0;
    f32 x_pos = x;
    f32 y_pos = y;
    
    while (*c != '\0')
    {        
        const u32 ci = (u32)*c;

        if (ci == '\n')
        {
            x_pos = x;
            y_pos -= atlas->line_gap * scale;
            c++;
            continue;
        }
        
        assert(ci >= atlas->start_charcode);
        assert(ci <= atlas->end_charcode);

        const u32 cis = ci - atlas->start_charcode; // correctly shifted index
        const Font_Glyph_Metric* metric = atlas->metrics + cis;
        
        if (ci == ' ')
        {
            x_pos += metric->advance_width * scale;
            c++;
            continue;
        }
        
        const f32 gw = (f32)atlas->font_size * scale;
        const f32 gh = (f32)atlas->font_size * scale;
        const f32 gx = x_pos + metric->offset_x * scale;
        const f32 gy = y_pos - (gh + metric->offset_y) * scale;
        
        mat4* transform = ctx->transforms + work_idx;
        identity(transform);
        translate(transform, vec3{gx, gy, 0.0f});
        ::scale(transform, vec3{gw, gh, 0.0f});

        ctx->charmap[work_idx] = cis;

        if (++work_idx >= FONT_RENDER_BATCH_SIZE)
        {
            glUniformMatrix4fv(glGetUniformLocation(ctx->program, "u_transforms"), work_idx, GL_FALSE, (f32*)ctx->transforms);
            glUniform1uiv(glGetUniformLocation(ctx->program, "u_charmap"), work_idx, ctx->charmap);
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, work_idx);
            work_idx = 0;
        }

        x_pos += metric->advance_width * scale;
        c++;
    }

    glUniformMatrix4fv(glGetUniformLocation(ctx->program, "u_transforms"), work_idx, GL_FALSE, (f32*)ctx->transforms);
    glUniform1uiv(glGetUniformLocation(ctx->program, "u_charmap"), work_idx, ctx->charmap);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, work_idx);
    
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void on_window_resize(const Font_Render_Context* ctx, s32 w, s32 h)
{
    glUseProgram(ctx->program);
    const mat4 projection = mat4_ortho(0.0f, (f32)w, 0.0f, (f32)h, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(ctx->program, "u_projection"), 1, GL_FALSE, (f32*)&projection);
    glUseProgram(0);
}

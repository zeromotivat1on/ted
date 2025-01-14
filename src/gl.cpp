#include "pch.h"
#include "gl.h"
#include "file.h"
#include "font.h"
#include "arena.h"
#include <stdio.h>
#include <glad/glad.h>
#include <vendor/stb_truetype.h>

u32 gl_create_program(const char* vs_src, const char* fs_src)
{
    static char info_log[1024];

    assert(vs_src);
    assert(fs_src);
    
    u32 vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_src, null);
    glCompileShader(vs);

    s32 success;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vs, sizeof(info_log), null, info_log);
        printf("Failed to compile vertex shader: %s\n", info_log);
        return -1;
    }
    
    u32 fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_src, null);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fs, sizeof(info_log), null, info_log);
        printf("Failed to compile fragment shader: %s\n", info_log);
        return -1;
    }

    u32 program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, sizeof(info_log), null, info_log);
        printf("Failed to link shader program: %s\n", info_log);
        return -1;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

u32 gl_load_program(Arena* arena, const char* vs_path, const char* fs_path)
{
    s32 vs_size = 0;
    const char* vs_src = (char*)read_entire_file(arena, vs_path, &vs_size);
    
    s32 fs_size = 0;
    const char* fs_src = (char*)read_entire_file(arena, fs_path, &fs_size);

    const u32 program = gl_create_program(vs_src, fs_src);

    pop(arena, fs_size);
    pop(arena, vs_size);

    return program;
}

GL_Text_Render_Context* gl_create_text_render_context(Arena* arena, const Font* font, u32 start_code, u32 end_code, u16 font_size, s32 window_w, s32 window_h)
{
    GL_Text_Render_Context* ctx = push_struct(arena, GL_Text_Render_Context);
    ctx->glyph_cache = gl_cache_glyph_bitmaps(arena, font, start_code, end_code, font_size);

    const f32 scale = stbtt_ScaleForPixelHeight(font->info, (f32)font_size);
    s32 ascent, descent, line_gap;
    stbtt_GetFontVMetrics(font->info, &ascent, &descent, &line_gap);
    ctx->line_gap = (ascent - descent + line_gap) * scale;

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

    ctx->program = gl_load_program(arena, DIR_SHADERS "text_2d.vs", DIR_SHADERS "text_2d.fs");
    glUseProgram(ctx->program);

    ctx->u_projection.name = "u_projection";
    ctx->u_projection.val = mat4_ortho(0.0f, (f32)window_w, 0.0f, (f32)window_h, -1.0f, 1.0f);

    ctx->u_transform.name = "u_transform";
    ctx->u_transform.val = mat4{0};

    ctx->u_text_color.name = "u_text_color";
    ctx->u_text_color.val = vec3{1.0f, 1.0f, 1.0f};
    
    return ctx;
}

GL_Glyph_Cache* gl_cache_glyph_bitmaps(Arena* arena, const Font* font, u32 start_code, u32 end_code, s16 font_size)
{
    GL_Glyph_Cache* cache = push_struct(arena, GL_Glyph_Cache);
    cache->start_code = start_code;
    cache->end_code = end_code;
    cache->glyphs = push_array(arena, end_code - start_code, GL_Glyph);
    
    for (u32 c = cache->start_code, i = 0; c <= cache->end_code; ++c, ++i)
    {
        GL_Glyph* glyph = cache->glyphs + i;
        glGenTextures(1, &glyph->texture);
    }
    
    gl_update_glyph_bitmaps(font, cache, font_size);
   
    return cache;
}

#include <malloc.h>

void gl_update_glyph_bitmaps(const Font* font, GL_Glyph_Cache* cache, s16 font_size)
{
    // stbtt rasterizes glyphs as 8bpp, so tell open gl to use 1 byte per color channel.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    const f32 scale = stbtt_ScaleForPixelHeight(font->info, (f32)font_size);

    u8* bitmap = (u8*)malloc(font_size * font_size);
    assert(bitmap);
        
    for (u32 c = cache->start_code, i = 0; c <= cache->end_code; ++c, ++i)
    {
        GL_Glyph* glyph = cache->glyphs + i;
        
        const s32 glyph_index = stbtt_FindGlyphIndex(font->info, c);
#if 0
        //u8* bitmap = stbtt_GetGlyphBitmap(font->info, scale, scale, glyph_index, &glyph->width_px, &glyph->height_px, &glyph->offset_x_px, &glyph->offset_y_px);
#else   
        u8* stb_bitmap = stbtt_GetGlyphBitmap(font->info, scale, scale, glyph_index, &glyph->width_px, &glyph->height_px, &glyph->offset_x_px, &glyph->offset_y_px);

        memset(bitmap, 0, font_size * font_size);

        // Get glyph bounding box
        int x0, y0, x1, y1;
        stbtt_GetGlyphBitmapBox(font->info, glyph_index, scale, scale, &x0, &y0, &x1, &y1);

        // Calculate position to center the glyph in the square
        int glyph_w = x1 - x0;
        int glyph_h = y1 - y0;
        int x_offset = (font_size - glyph_w) / 2; // Center horizontally
        int y_offset = (font_size - glyph_h) / 2; // Center vertically

        // Copy glyph bitmap into the center of the larger square
        for (int y = 0; y < glyph_h; ++y) {
            for (int x = 0; x < glyph_w; ++x) {
                int src_index = y * glyph_w + x;
                int dest_index = (y + y_offset) * font_size + (x + x_offset);
                if (dest_index >= 0 && dest_index < font_size * font_size)
                    bitmap[dest_index] = stb_bitmap[src_index];
            }
        }
        
        glyph->width_px = font_size;
        glyph->height_px = font_size;
        glyph->offset_x_px -= x_offset;
        glyph->offset_y_px -= y_offset;
#endif
        
        glBindTexture(GL_TEXTURE_2D, glyph->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, glyph->width_px, glyph->height_px, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
        
        stbtt_FreeBitmap(stb_bitmap, null);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        s32 advance_width;
        stbtt_GetGlyphHMetrics(font->info, glyph_index, &advance_width, 0);
        glyph->advance_width_px = (s32)(advance_width * scale);
    }

    free(bitmap);
    
    // Restore default color channel size.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void gl_render_text(GL_Text_Render_Context* ctx, vec2 pos, const char* text)
{
    glUseProgram(ctx->program);
    glBindVertexArray(ctx->vao);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
 
    const vec3 color = ctx->u_text_color.val;
    glUniform3f(glGetUniformLocation(ctx->program, ctx->u_text_color.name), color.r, color.g, color.b);
    glUniformMatrix4fv(glGetUniformLocation(ctx->program, ctx->u_projection.name), 1, GL_FALSE, (f32*)&ctx->u_projection.val);
        
    glActiveTexture(GL_TEXTURE0);

    vec2 def_pos = pos;
    const char* c = text;
    while (*c != '\0')
    {
        const u32 ci = (u32)*c;

        if (ci == '\n')
        {
            pos.y -= ctx->line_gap;
            pos.x = def_pos.x;
            c++;
            continue;
        }
                
        assert(ci >= ctx->glyph_cache->start_code);
        assert(ci <= ctx->glyph_cache->end_code);
        
        GL_Glyph* glyph = ctx->glyph_cache->glyphs + (ci - ctx->glyph_cache->start_code);

        if (*c == ' ')
        {
            pos.x += glyph->advance_width_px;
            c++;
            continue;
        }
        
        const f32 x = pos.x + glyph->offset_x_px;
        const f32 y = pos.y - (glyph->height_px + glyph->offset_y_px);
        const f32 w = (f32)glyph->width_px;
        const f32 h = (f32)glyph->height_px;
        
        identity(&ctx->u_transform.val);
        translate(&ctx->u_transform.val, vec3{x, y, 0.0f});
        scale(&ctx->u_transform.val, vec3{w, h, 0.0f});
        glUniformMatrix4fv(glGetUniformLocation(ctx->program, ctx->u_transform.name), 1, GL_FALSE, (f32*)&ctx->u_transform.val);
        
        glBindTexture(GL_TEXTURE_2D, glyph->texture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        pos.x += glyph->advance_width_px;
        c++;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

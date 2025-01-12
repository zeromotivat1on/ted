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
    
    glGenVertexArrays(1, &ctx->vao);
    glGenBuffers(1, &ctx->vbo);
    
    glBindVertexArray(ctx->vao);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
    
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(f32), null, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    ctx->program = gl_load_program(arena, DIR_SHADERS "text_2d.vs", DIR_SHADERS "text_2d.fs");
    glUseProgram(ctx->program);

    ctx->u_projection.name = "u_projection";
    ctx->u_projection.val = mat4_ortho(0.0f, (f32)window_w, 0.0f, (f32)window_h, -1.0f, 1.0f);

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

void gl_update_glyph_bitmaps(const Font* font, GL_Glyph_Cache* cache, s16 font_size)
{
    // stbtt rasterizes glyphs as 8bpp, so tell open gl to use 1 byte per color channel.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    const f32 scale = stbtt_ScaleForPixelHeight(font->info, (f32)font_size);

    for (u32 c = cache->start_code, i = 0; c <= cache->end_code; ++c, ++i)
    {
        GL_Glyph* glyph = cache->glyphs + i;
        
        const s32 glyph_index = stbtt_FindGlyphIndex(font->info, c);
        u8* bitmap = stbtt_GetGlyphBitmap(font->info, 0, scale, glyph_index, &glyph->width_px, &glyph->height_px, &glyph->offset_x_px, &glyph->offset_y_px);

        glBindTexture(GL_TEXTURE_2D, glyph->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, glyph->width_px, glyph->height_px, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);

        stbtt_FreeBitmap(bitmap, null);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        s32 advance_width;
        stbtt_GetGlyphHMetrics(font->info, glyph_index, &advance_width, 0);
        glyph->advance_width_px = (s32)(advance_width * scale);
    }
    
    // Restore default color channel size.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void gl_render_text(const GL_Text_Render_Context* ctx, vec2 pos, const char* text)
{
    glUseProgram(ctx->program);
    glBindVertexArray(ctx->vao);

    const vec3 color = ctx->u_text_color.val;
    glUniform3f(glGetUniformLocation(ctx->program, ctx->u_text_color.name), color.r, color.g, color.b);
    glUniformMatrix4fv(glGetUniformLocation(ctx->program, ctx->u_projection.name), 1, GL_FALSE, (f32*)&ctx->u_projection.val);
        
    glActiveTexture(GL_TEXTURE0);

    const char* c = text;
    while (*c != '\0')
    {
        const u32 ci = (u32)*c;
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

        const f32 vertices[6][4] = {
            { x,     y + h, 0.0f, 0.0f },            
            { x,     y,     0.0f, 1.0f },
            { x + w, y,     1.0f, 1.0f },

            { x,     y + h, 0.0f, 0.0f },
            { x + w, y,     1.0f, 1.0f },
            { x + w, y + h, 1.0f, 0.0f },
        };

        glBindTexture(GL_TEXTURE_2D, glyph->texture);
            
        glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        pos.x += glyph->advance_width_px;
        c++;
    }
}

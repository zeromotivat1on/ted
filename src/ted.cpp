#include "pch.h"
#include <math.h>
#include <stdio.h>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "ted.h"
#include "file.h"
#include "font.h"
#include "arena.h"
#include "matrix.h"
#include "profile.h"
#include "gap_buffer.h"

static void framebuffer_size_callback(GLFWwindow* window, s32 width, s32 height)
{
    auto* ctx = (Ted_Context*)glfwGetWindowUserPointer(window);
    on_framebuffer_resize(ctx->render_ctx, width, height);
    glViewport(0, 0, width, height);
}

static void window_size_callback(GLFWwindow* window, s32 width, s32 height)
{
    auto* ctx = (Ted_Context*)glfwGetWindowUserPointer(window);

    // @Cleanup: not sure if its a good idea to loop through all.
    for (s16 i = 0; i < ctx->buffer_count; ++i)
        ctx->buffers[i].y += height - ctx->window_h;

    ctx->window_w = width;
    ctx->window_h = height;
}

static void char_callback(GLFWwindow* window, u32 character)
{
    //printf("Window char (%c) as key (%u)\n", character, character);

    auto* ctx = (Ted_Context*)glfwGetWindowUserPointer(window);
    auto* buffer = ctx->buffers + ctx->active_buffer_idx;
    push_char(buffer->display_buffer, (char)character);
}

static void key_callback(GLFWwindow* window, s32 key, s32 scancode, s32 action, s32 mods)
{
    //printf("Window key (%d) as char (%c)\n", key, key);

    auto* ctx = (Ted_Context*)glfwGetWindowUserPointer(window);
    auto* buffer = active_buffer(ctx);
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
  
    if (key == GLFW_KEY_ENTER && (action == GLFW_PRESS || action == GLFW_REPEAT))
        push_char(buffer->display_buffer, '\n');

    if (key == GLFW_KEY_BACKSPACE && (action == GLFW_PRESS || action == GLFW_REPEAT))
        delete_char(buffer->display_buffer);
    
    if (key == GLFW_KEY_DELETE && (action == GLFW_PRESS || action == GLFW_REPEAT))
        delete_char_overwrite(buffer->display_buffer);

    if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT) && !(mods & GLFW_MOD_ALT))
        move_cursor(buffer->display_buffer, -1);
        
    if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT) && !(mods & GLFW_MOD_ALT))
        move_cursor(buffer->display_buffer, 1);

    if (key == GLFW_KEY_EQUAL && (action == GLFW_PRESS || action == GLFW_REPEAT) && mods & GLFW_MOD_CONTROL)
        increase_font_size(ctx);

    if (key == GLFW_KEY_MINUS && (action == GLFW_PRESS || action == GLFW_REPEAT) && mods & GLFW_MOD_CONTROL)
        decrease_font_size(ctx);

     if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT) && mods & GLFW_MOD_ALT)
        open_next_buffer(ctx);

     if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT) && mods & GLFW_MOD_ALT)
        open_prev_buffer(ctx);
}

static void scroll_callback(GLFWwindow* window, f64 xoffset, f64 yoffset)
{    
    auto* ctx = (Ted_Context*)glfwGetWindowUserPointer(window);
    auto* atlas = active_atlas(ctx);
    auto* buffer = active_buffer(ctx);

    if (yoffset)
    {
        // Horizontal scroll in case of pressed shift.
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            buffer->x += (s16)yoffset * atlas->font_size;
        else
            buffer->y -= (s16)yoffset * atlas->new_line_offset;
    }
}

static void drop_callback(GLFWwindow* window, s32 count, const char** paths)
{
    SCOPE_TIMER("drop_callback");
    
    auto* ctx = (Ted_Context*)glfwGetWindowUserPointer(window);
    s16 buffer_idx = 0;
    for (s32 i = 0; i < count; ++i)
    {
         // @Cleanup: maybe its a bit annoying to write such procedure every time
         // you want to open a new file and we should tweak api to have just one
         // function that opens file in new buffer.
         buffer_idx = create_buffer(ctx);
         load_file_contents(ctx, buffer_idx, paths[i]);

         auto* buffer = ctx->buffers + buffer_idx;
         set_cursor(buffer->display_buffer, 0);
         strcpy(buffer->path, paths[i]);
    }

    set_active_buffer(ctx, buffer_idx);
}

Ted_Buffer* active_buffer(Ted_Context* ctx)
{
    assert(ctx->active_buffer_idx < ctx->buffer_count);
    return ctx->buffers + ctx->active_buffer_idx;
}

Font_Atlas* active_atlas(Ted_Context* ctx)
{
    assert(ctx->active_atlas_idx < ctx->atlas_count);
    return ctx->atlases + ctx->active_atlas_idx;
}

void init_ted_context(Ted_Context* ctx, void* memory, u32 size)
{
    *ctx = {0};
    ctx->arena = create_arena(memory, size);
    ctx->font = push_struct(&ctx->arena, Font);
    ctx->atlases = push_array(&ctx->arena, TED_MAX_ATLASES, Font_Atlas);
    ctx->render_ctx = push_struct(&ctx->arena, Font_Render_Context);
    ctx->buffers = push_array(&ctx->arena, TED_MAX_BUFFERS, Ted_Buffer);
    ctx->bg_color = vec3{2.0f / 255.0f, 26.0f / 255.0f, 25.0f / 255.0f};
    ctx->text_color = vec3{255.0f / 255.0f, 220.0f / 255.0f, 194.0f / 255.0f};
    ctx->buffer_max_x = 4; // @Todo: make it customizable constant.

#if TED_DEBUG
    ctx->debug_atlas = push_struct(&ctx->arena, Font_Atlas);
#endif
}

void destroy(Ted_Context* ctx)
{    
    // @Cleanup: these frees should not be here after gap buffer will use memory arena.
    for (s16 i = 0; i < ctx->buffer_count; ++i)
        free(ctx->buffers[i].display_buffer);

    clear(&ctx->arena);
    glfwTerminate();
}

bool alive(Ted_Context* ctx)
{
    assert(ctx->window);
    return !glfwWindowShouldClose(ctx->window);
}

void create_window(Ted_Context* ctx, s16 w, s16 h, s16 x, s16 y)
{
    assert(!ctx->window);
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    ctx->window = glfwCreateWindow(w, h, "", null, null);
    if (!ctx->window)
    {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return;
    }

    ctx->window_w = w;
    ctx->window_h = h;

    glfwSetWindowPos(ctx->window, x, y);
    glfwSetWindowUserPointer(ctx->window, ctx);
    
    glfwMakeContextCurrent(ctx->window);
    glfwSwapInterval(0);

    glfwSetWindowSizeCallback(ctx->window, window_size_callback);
    glfwSetFramebufferSizeCallback(ctx->window, framebuffer_size_callback);
    glfwSetCharCallback(ctx->window, char_callback);
    glfwSetKeyCallback(ctx->window, key_callback);
    glfwSetScrollCallback(ctx->window, scroll_callback);
    glfwSetDropCallback(ctx->window, drop_callback);
}

void load_font(Ted_Context* ctx, const char* path)
{
    init_font(&ctx->arena, ctx->font, path);
}

void init_render_context(Ted_Context* ctx)
{
    assert(ctx->window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    init_font_render_context(&ctx->arena, ctx->render_ctx, ctx->window_w, ctx->window_h);
}

void bake_font(Ted_Context* ctx, u32 start_charcode, u32 end_charcode, s16 min_font_size, s16 max_font_size, s16 font_size_stride)
{
    s16 i = 0;
    for (s16 font_size = min_font_size; font_size <= max_font_size; font_size += font_size_stride, ++i)
    {
        if (i >= TED_MAX_ATLASES) break;
        bake_font_atlas(&ctx->arena, ctx->font, ctx->atlases + i, start_charcode, end_charcode, font_size);
    }

    ctx->atlas_count = i;
    ctx->active_atlas_idx = i / 2;

#if TED_DEBUG
    bake_font_atlas(&ctx->arena, ctx->font, ctx->debug_atlas, 0, 127, 16);
#endif
}

s16 create_buffer(Ted_Context* ctx)
{
    if (ctx->buffer_count > TED_MAX_BUFFERS) return -1;

    auto* atlas = active_atlas(ctx);
    auto* buffer = ctx->buffers + ctx->buffer_count;
    buffer->arena = subarena(&ctx->arena, TED_MAX_BUFFER_SIZE);
    buffer->display_buffer = push_struct(&ctx->arena, Gap_Buffer);
    buffer->path = push_array(&ctx->arena, 128, char);
    buffer->x = ctx->buffer_max_x;
    
    // @Cleanup: pass arena or smth.
    init_gap_buffer(buffer->display_buffer, 128);
    
    return ctx->buffer_count++;
}

void load_file_contents(Ted_Context* ctx, s16 buffer_idx, const char* path)
{
    if (buffer_idx >= ctx->buffer_count) return;

    auto* buffer = ctx->buffers + buffer_idx;
    clear(&buffer->arena);
    
    s32 size = 0;
    u8* data = read_entire_file(&buffer->arena, path, &size);

    // @Todo: handle non-ascii?
    push_str(buffer->display_buffer, (char*)data, size);
}

void kill_buffer(Ted_Context* ctx, s16 buffer_idx)
{
    if (buffer_idx >= ctx->buffer_count) return;

    auto* buffer = ctx->buffers + buffer_idx;
    clear(&buffer->arena);
    // @Cleanup: this gap buffer free should be removed as arena must handle buffer memory.
    free(buffer->display_buffer);
}

void set_active_buffer(Ted_Context* ctx, s16 buffer_idx)
{
    if (buffer_idx >= ctx->buffer_count) return;
    ctx->active_buffer_idx = buffer_idx;

    glfwSetWindowTitle(ctx->window, active_buffer(ctx)->path);
}

void open_next_buffer(Ted_Context* ctx)
{
    ctx->active_buffer_idx++;
    if (ctx->active_buffer_idx >= ctx->buffer_count)
        ctx->active_buffer_idx = 0;

    glfwSetWindowTitle(ctx->window, active_buffer(ctx)->path);
}

void open_prev_buffer(Ted_Context* ctx)
{
    ctx->active_buffer_idx--;
    if (ctx->active_buffer_idx < 0)
        ctx->active_buffer_idx = ctx->buffer_count - 1;

    glfwSetWindowTitle(ctx->window, active_buffer(ctx)->path);
}

// @Fixme
void increase_font_size(Ted_Context* ctx)
{
    ctx->active_atlas_idx = min(ctx->atlas_count - 1, ctx->active_atlas_idx + 1);
}

// @Fixme
void decrease_font_size(Ted_Context* ctx)
{
    ctx->active_atlas_idx = max(0, ctx->active_atlas_idx - 1);
}

static void render_batch_glyphs(Ted_Context* ctx, s32 count)
{
    glUniformMatrix4fv(glGetUniformLocation(ctx->render_ctx->program, "u_transforms"), count, GL_FALSE, (f32*)ctx->render_ctx->transforms);
    glUniform1uiv(glGetUniformLocation(ctx->render_ctx->program, "u_charmap"), count, ctx->render_ctx->charmap);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
}

static void render(Ted_Context* ctx)
{
    auto* buffer = active_buffer(ctx);
    const auto* atlas = active_atlas(ctx);

    glUseProgram(ctx->render_ctx->program);
    glBindVertexArray(ctx->render_ctx->vao);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->render_ctx->vbo);
    glBindTexture(GL_TEXTURE_2D_ARRAY, atlas->texture_array);
    
    glActiveTexture(GL_TEXTURE0);
    glUniform3f(glGetUniformLocation(ctx->render_ctx->program, "u_text_color"), ctx->text_color.r, ctx->text_color.g, ctx->text_color.b);
    
    const s32 buffer_data_size = (s32)data_size(buffer->display_buffer);
    const s32 prefix_size = (s32)prefix_data_size(buffer->display_buffer);
    
    s16 work_idx = 0;
    s32 x = buffer->x;
    s32 y = buffer->y;

    buffer->line_count = 0;
    for (s32 i = 0, j = 0; i < buffer_data_size; ++i)
    {
        if (y < 0) break;
                
        // @Cleanup: looks nasty, refactor.
        char c;
        if (i < prefix_size) c = buffer->display_buffer->start[i];
        else c = buffer->display_buffer->gap_end[j++];

        // @Cleanup: super straightforward text culling,
        // don't like it, but it gets the job done for now, refactor later.
        if (y > ctx->window_h)
        {
            if (c == '\n')
            {
                y -= atlas->new_line_offset;
                buffer->line_count++;
            }
            
            continue;
        }

        if (c == '\n')
        {
            x = buffer->x;
            y -= atlas->new_line_offset;
            buffer->line_count++;
            continue;
        }
        
        assert((u32)c >= atlas->start_charcode);
        assert((u32)c <= atlas->end_charcode);

        const u32 ci = c - atlas->start_charcode; // correctly shifted index
        const Font_Glyph_Metric* metric = atlas->metrics + ci;

        if (c == ' ')
        {
            x += metric->advance_width;
            continue;
        }

        if (c == '\t')
        {
            // @Todo: handle different tab sizes, 4 by default for now.
            x += 4 * metric->advance_width;
            continue;
        }
                
        const f32 gw = (f32)atlas->font_size;
        const f32 gh = (f32)atlas->font_size;
        const f32 gx = (f32)(x + metric->offset_x);
        const f32 gy = y - (gh + metric->offset_y);
        
        mat4* transform = ctx->render_ctx->transforms + work_idx;
        identity(transform);
        translate(transform, vec3{gx, gy, 0.0f});
        scale(transform, vec3{gw, gh, 0.0f});

        ctx->render_ctx->charmap[work_idx] = ci;

        if (++work_idx >= FONT_RENDER_BATCH_SIZE)
        {
            render_batch_glyphs(ctx, work_idx);
            work_idx = 0;
        }

        x += metric->advance_width;
    }
    
    if (work_idx > 0) render_batch_glyphs(ctx, work_idx);
    
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

static s32 vert_offset_from_baseline(const Font* font, const Font_Atlas* atlas)
{
    return (s32)((font->ascent + font->line_gap) * atlas->px_h_scale);
}

static void copy(u32* dst, const char* src, s32 size)
{
    for (s32 i = 0; i < size; ++i) dst[i] = src[i];    
}

void update_frame(Ted_Context* ctx)
{
    static f32 prev_time = (f32)glfwGetTime();

    const auto* atlas = active_atlas(ctx);
    // @Cleanup: calculate only on window resize?
    ctx->buffer_min_y = ctx->window_h - vert_offset_from_baseline(ctx->font, atlas);

    // @Todo: update all opened buffers (feature to come).
    auto* buffer = active_buffer(ctx);
    buffer->min_x = -ctx->window_w; // @Todo: should be equal to longest line length
    buffer->max_y = ctx->buffer_min_y + (buffer->line_count * atlas->new_line_offset);
    buffer->x = clamp(buffer->x, buffer->min_x, ctx->buffer_max_x);
    buffer->y = clamp(buffer->y, ctx->buffer_min_y, buffer->max_y);
    
    glClearColor(ctx->bg_color.r, ctx->bg_color.g, ctx->bg_color.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    render(ctx);

#if TED_DEBUG
    static char debug_str[512];
    static u32 debug_text[512];
    
    s32 debug_str_size = sprintf(debug_str, "%.2fms %.ffps", ctx->dt * 1000.0f, 1 / ctx->dt);
    copy(debug_text, debug_str, debug_str_size);

    // @Cleanup: using line gap for x positioning is not the best idea.
    f32 x = ctx->window_w - ctx->debug_atlas->new_line_offset * debug_str_size * 0.5f;
    f32 y = (f32)(ctx->window_h - ctx->debug_atlas->new_line_offset);
    render_text(ctx->render_ctx, ctx->debug_atlas, debug_text, debug_str_size, 1.0f, x, y, 1.0f, 1.0f, 1.0f);

    debug_str_size = sprintf(debug_str, "cursor_pos=%lld\nend=%lld\ngap_start=%lld\ngap_end=%lld\nxy=(%d, %d)\nmin_xy=(%d, %d)\nmax_xy=(%d, %d)\nfont_size=%d",
                             cursor_pos(buffer->display_buffer),
                             total_data_size(buffer->display_buffer),
                             prefix_data_size(buffer->display_buffer),
                             buffer->display_buffer->gap_end - buffer->display_buffer->start,
                             buffer->x, buffer->y, buffer->min_x, ctx->buffer_min_y, ctx->buffer_max_x, buffer->max_y, atlas->font_size);
    copy(debug_text, debug_str, debug_str_size);

    x = ctx->window_w - ctx->debug_atlas->font_size * 12.0f;
    y -= ctx->debug_atlas->new_line_offset;
    render_text(ctx->render_ctx, ctx->debug_atlas, debug_text, debug_str_size, 1.0f, x, y, 1.0f, 1.0f, 1.0f);
#endif
    
    glfwSwapBuffers(ctx->window);
    glfwPollEvents();

    const f32 time = (f32)glfwGetTime();
    ctx->dt = time - prev_time;
    prev_time = time;
}

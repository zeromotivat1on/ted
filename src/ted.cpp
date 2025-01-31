#include "pch.h"
#include <math.h>
#include <stdio.h>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "ted.h"
#include "gl.h"
#include "font.h"
#include "arena.h"
#include "matrix.h"
#include "profile.h"
#include "settings.h"

static void on_framebuffer_resize(u32 program, s32 w, s32 h)
{
    glUseProgram(program);
    const mat4 projection = mat4_ortho(0.0f, (f32)w, 0.0f, (f32)h, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(program, "u_projection"), 1, GL_FALSE, (f32*)&projection);
    glUseProgram(0);
}

static void framebuffer_size_callback(GLFWwindow* window, s32 width, s32 height)
{
    auto* ctx = (Ted_Context*)glfwGetWindowUserPointer(window);
    on_framebuffer_resize(ctx->font_render_ctx->program, width, height);
    on_framebuffer_resize(ctx->cursor_render_ctx->program, width, height);
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
    push_char(ctx, ctx->active_buffer_idx, (char)character);
}

static void overwrite_file(Arena* arena, const Ted_Buffer* buffer)
{
    const s32 buffer_data_size = data_size(&buffer->display_buffer);
    char* utf8 = push_array(arena, buffer_data_size, char);
    fill_utf8(&buffer->display_buffer, utf8);
    overwrite_file(buffer->path, (u8*)utf8, buffer_data_size);
}

static void key_callback(GLFWwindow* window, s32 key, s32 scancode, s32 action, s32 mods)
{
    //printf("Window key (%d) as char (%c)\n", key, key);

    auto* ctx = (Ted_Context*)glfwGetWindowUserPointer(window);
    const s16 buffer_idx = ctx->active_buffer_idx;
    auto* buffer = ctx->buffers + buffer_idx;

    switch (key)
    {
    case GLFW_KEY_ESCAPE: 
        if (action == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
        break;

    case GLFW_KEY_TAB:
        if (action == GLFW_PRESS) push_str(ctx, buffer_idx, tab_as_space_string(), ted_settings.tab_size);
        break;

    case GLFW_KEY_HOME:
        if (action == GLFW_PRESS)
        {
            if (mods & GLFW_MOD_CONTROL) set_cursor(ctx, buffer_idx, 0, 0);
            else set_cursor(ctx, buffer_idx, buffer->cursor.row, 0);
        }
        
        break;

    case GLFW_KEY_END:
        if (action == GLFW_PRESS)
        {
            if (mods & GLFW_MOD_CONTROL)
                set_cursor(ctx, buffer_idx, buffer->last_line_idx, buffer->line_lengths[buffer->last_line_idx]);    
            else
                set_cursor(ctx, buffer_idx, buffer->cursor.row, buffer->line_lengths[buffer->cursor.row]);
        }
        
        break;
        
    case GLFW_KEY_S:
        if (action == GLFW_PRESS && mods & GLFW_MOD_CONTROL)
            overwrite_file(&ctx->arena, buffer);
        break;
        
    case GLFW_KEY_ENTER:
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
            push_char(ctx, buffer_idx, '\n');
        break;
        
    case GLFW_KEY_BACKSPACE:
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
            delete_char(ctx, buffer_idx);
        break;
        
    case GLFW_KEY_DELETE:
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
            delete_char_overwrite(ctx, buffer_idx);
        break;
        
    case GLFW_KEY_LEFT:
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {
            if (mods & GLFW_MOD_ALT) open_prev_buffer(ctx);
            else move_cursor_horizontally(ctx, buffer_idx, -1);
        }
        
        break;
        
    case GLFW_KEY_RIGHT:
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {
            if (mods & GLFW_MOD_ALT) open_next_buffer(ctx);
            else move_cursor_horizontally(ctx, buffer_idx, 1);
        }
        
        break;

    case GLFW_KEY_UP:
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
            move_cursor_vertically(ctx, buffer_idx, -1);
        break;

    case GLFW_KEY_DOWN:
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
            move_cursor_vertically(ctx, buffer_idx, 1);
        break;
        
    case GLFW_KEY_EQUAL:
        if ((action == GLFW_PRESS || action == GLFW_REPEAT) && mods & GLFW_MOD_CONTROL)
            increase_font_size(ctx);
        break;
        
    case GLFW_KEY_MINUS:
        if ((action == GLFW_PRESS || action == GLFW_REPEAT) && mods & GLFW_MOD_CONTROL)
            decrease_font_size(ctx);
        break;
    }
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
            buffer->y -= (s16)yoffset * atlas->line_height;
    }
}

static void drop_callback(GLFWwindow* window, s32 count, const char** paths)
{
    SCOPE_TIMER(__FUNCTION__);
    
    auto* ctx = (Ted_Context*)glfwGetWindowUserPointer(window);
    s16 buffer_idx = 0;
    for (s32 i = 0; i < count; ++i)
    {
         // @Cleanup: maybe its a bit annoying to write such procedure every time
         // you want to open a new file and we should tweak api to have just one
         // function that opens file in new buffer.
         buffer_idx = create_buffer(ctx);
         load_file_contents(ctx, buffer_idx, paths[i]);
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
    ctx->font_render_ctx = push_struct(&ctx->arena, Font_Render_Context);
    ctx->cursor_render_ctx = push_struct(&ctx->arena, Ted_Cursor_Render_Context);
    ctx->atlases = push_array(&ctx->arena, TED_MAX_ATLASES, Font_Atlas);
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
        free(&ctx->buffers[i].display_buffer);

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
    init_font(ctx->font, &ctx->arena, path);
}

static void init_cursor_render_context(Ted_Cursor_Render_Context* ctx, Arena* arena)
{
    ctx->program = gl_load_program(arena, DIR_SHADERS "cursor.vs", DIR_SHADERS "cursor.fs");

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

    init_font_render_context(ctx->font_render_ctx, &ctx->arena, ctx->window_w, ctx->window_h);
    init_cursor_render_context(ctx->cursor_render_ctx, &ctx->arena);

    on_framebuffer_resize(ctx->font_render_ctx->program, ctx->window_w, ctx->window_h);
    on_framebuffer_resize(ctx->cursor_render_ctx->program, ctx->window_w, ctx->window_h);
}

void bake_font(Ted_Context* ctx, u32 start_charcode, u32 end_charcode, s16 min_font_size, s16 max_font_size, s16 font_size_stride)
{
    s16 i = 0;
    for (s16 font_size = min_font_size; font_size <= max_font_size; font_size += font_size_stride, ++i)
    {
        if (i >= TED_MAX_ATLASES) break;
        bake_font_atlas(ctx->atlases + i, &ctx->arena, ctx->font, start_charcode, end_charcode, font_size);
    }

    ctx->atlas_count = i;
    ctx->active_atlas_idx = i / 2;

#if TED_DEBUG
    bake_font_atlas(ctx->debug_atlas, &ctx->arena, ctx->font, 0, 127, 16);
#endif
}

s16 create_buffer(Ted_Context* ctx)
{
    if (ctx->buffer_count > TED_MAX_BUFFERS) return -1;

    auto* atlas = active_atlas(ctx);
    auto* buffer = ctx->buffers + ctx->buffer_count;
    buffer->arena = subarena(&ctx->arena, TED_MAX_BUFFER_SIZE);
    buffer->path = push_array(&buffer->arena, 256, char);
    buffer->line_lengths = push_array(&buffer->arena, TED_MAX_LINE_COUNT, s32);
    buffer->x = ctx->buffer_max_x;

    strcpy(buffer->path, "dummy");
        
    // @Cleanup: pass arena or smth.
    init_gap_buffer(&buffer->display_buffer, 128);

    return ctx->buffer_count++;
}

void load_file_contents(Ted_Context* ctx, s16 buffer_idx, const char* path)
{
    assert(buffer_idx < ctx->buffer_count);

    auto* buffer = ctx->buffers + buffer_idx;
    strcpy(buffer->path, path);

    s32 size = 0; // includes null-termination character
    u8* data = read_entire_file(&buffer->arena, path, &size);

    // @Todo: handle non-ascii?
    push_str(ctx, buffer_idx, (char*)data, size - 1);
    set_cursor(ctx, buffer_idx, 0, 0);
}

void kill_buffer(Ted_Context* ctx, s16 buffer_idx)
{
    assert(buffer_idx < ctx->buffer_count);

    auto* buffer = ctx->buffers + buffer_idx;
    clear(&buffer->arena);
    // @Cleanup: this gap buffer free should be removed as arena must handle buffer memory.
    free(&buffer->display_buffer);
}

void set_active_buffer(Ted_Context* ctx, s16 buffer_idx)
{
    assert(buffer_idx < ctx->buffer_count);
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

static void insert_line(Ted_Buffer* buffer, s32 idx, s32 line_length)
{
    buffer->last_line_idx++;
    for (s32 i = buffer->last_line_idx; i > idx; --i)
        buffer->line_lengths[i] = buffer->line_lengths[i - 1];

    buffer->line_lengths[idx] = line_length;
}

static void remove_line(Ted_Buffer* buffer, s32 idx)
{
    for (s32 i = idx; i <= buffer->last_line_idx; ++i)
        buffer->line_lengths[i] = buffer->line_lengths[i + 1];

    buffer->line_lengths[buffer->last_line_idx] = 0;
    buffer->last_line_idx--;
}

void push_char(Ted_Context* ctx, s16 buffer_idx, char c)
{
    assert(buffer_idx < ctx->buffer_count);
    
    auto* buffer = ctx->buffers + buffer_idx;
    push_char(&buffer->display_buffer, c);
    
    if (c == '\n')
    {   
        if (buffer->last_line_idx >= TED_MAX_LINE_COUNT)
        {
            printf("Reached max line count (%d)\n", TED_MAX_LINE_COUNT);
            return;
        }

        const s32 right_line_part_length = buffer->line_lengths[buffer->cursor.row] - buffer->cursor.col;
        
        buffer->line_lengths[buffer->cursor.row] -= right_line_part_length;
        insert_line(buffer, buffer->cursor.row + 1, right_line_part_length);

        buffer->cursor.row++;
        buffer->cursor.col = 0;
    }
    else
    {
        buffer->cursor.col++;
        buffer->line_lengths[buffer->cursor.row] += 1;
    }    
}

void push_str(Ted_Context* ctx, s16 buffer_idx, const char* str, s32 size)
{
    // @Cleanup: brute-force implementation for now.
    // @Speed: this will be very slow if string has several lines.
    for (s32 i = 0; i < size; ++i)
        push_char(ctx, buffer_idx, str[i]);
}

void delete_char(Ted_Context* ctx, s16 buffer_idx)
{
    assert(buffer_idx < ctx->buffer_count);
    
    auto* buffer = ctx->buffers + buffer_idx;
    auto* display_buffer = &buffer->display_buffer;

    const char c_deleted = delete_char(display_buffer);
    if (c_deleted == '\n')
    {   
        const s32 deleted_line_length = buffer->line_lengths[buffer->cursor.row];
        const s32 prev_line_length = buffer->line_lengths[buffer->cursor.row - 1];
        buffer->line_lengths[buffer->cursor.row - 1] += deleted_line_length;
        remove_line(buffer, buffer->cursor.row);
 
        buffer->cursor.row--;
        buffer->cursor.col = prev_line_length;
    }
    else if (c_deleted != INVALID_CHAR)
    {
        buffer->cursor.col--;
        buffer->line_lengths[buffer->cursor.row] -= 1;
    }
}

void delete_char_overwrite(Ted_Context* ctx, s16 buffer_idx)
{
    assert(buffer_idx < ctx->buffer_count);
    
    auto* buffer = ctx->buffers + buffer_idx;
    auto* display_buffer = &buffer->display_buffer;

    const char c_deleted = delete_char_overwrite(display_buffer);

    if (c_deleted == '\n')
    {
        const s32 deleted_line_length = buffer->line_lengths[buffer->cursor.row + 1];
        buffer->line_lengths[buffer->cursor.row] += deleted_line_length;
        remove_line(buffer, buffer->cursor.row + 1);
    }
    else if (c_deleted != INVALID_CHAR)
    {
        buffer->line_lengths[buffer->cursor.row] -= 1;
    }
}

// Set cursor position and update gap buffer pointer according to new cursor.
void set_cursor(Ted_Context* ctx, s16 buffer_idx, s32 row, s32 col)
{
    assert(buffer_idx < ctx->buffer_count);

    auto* buffer = ctx->buffers + buffer_idx;
    auto* display_buffer = &buffer->display_buffer;

    if (row < 0 || row > buffer->last_line_idx)
    {
        printf("Incorrect cursor row position (%d)\n", row);
        return;
    }

    const s32 line_length = buffer->line_lengths[row];
    if (col < 0 || col > line_length)
    {
        printf("Incorrect cursor col position (%d)\n", col);
        return;
    }

    s32 pos = 0;
    for (s32 i = 0; i < row; ++i)
        pos += buffer->line_lengths[i] + 1; // include '\n' for pointer position
    pos += col;
    
    set_pointer(display_buffer, pos);
    
    buffer->cursor.row = row;
    buffer->cursor.col = col;
}

void move_cursor_horizontally(Ted_Context* ctx, s16 buffer_idx, s32 delta)
{
    assert(buffer_idx < ctx->buffer_count);
    const auto* buffer = ctx->buffers + buffer_idx;

    s32 new_row = buffer->cursor.row;
    s32 new_col = buffer->cursor.col + delta;
    
    const s32 current_line_length = buffer->line_lengths[buffer->cursor.row];
    if (new_col > current_line_length)
    {
        if (++new_row > buffer->last_line_idx) return;
        new_col -= (current_line_length + 1); // include '\n'
    }
    else if (new_col < 0)
    {
        if (--new_row < 0) return;
        new_col += buffer->line_lengths[new_row] + 1; // include '\n'
    }
    
    set_cursor(ctx, buffer_idx, new_row, new_col);
}

void move_cursor_vertically(Ted_Context* ctx, s16 buffer_idx, s32 delta)
{
    assert(buffer_idx < ctx->buffer_count);
    auto* buffer = ctx->buffers + buffer_idx;

    const s32 new_line_idx = buffer->cursor.row + delta;
    if (new_line_idx < 0 || new_line_idx > buffer->last_line_idx) return;
    
    const s32 current_line_length = buffer->line_lengths[buffer->cursor.row];
    buffer->cursor.col = clamp(buffer->cursor.col, 0, buffer->line_lengths[new_line_idx]);
    
    set_cursor(ctx, buffer_idx, new_line_idx, buffer->cursor.col);
}

static s32 line_width_px_till_pointer(const Font_Atlas* atlas, const Gap_Buffer* buffer, s32 start_pos)
{
    s32 width = 0;
    const s32 end_pos = pointer_pos(buffer);
    for (s32 i = start_pos; i < end_pos; ++i)
    {
        const char c = char_at(buffer, i);
        if (c != INVALID_CHAR)
        {
            const u32 ci = c - atlas->start_charcode; // correctly shifted index
            const Font_Glyph_Metric* metric = atlas->metrics + ci;
            width += metric->advance_width;
        }
    }
    return width;
}

static s32 line_start_pointer_pos(const Ted_Buffer* buffer)
{
    s32 pos = 0;
    for (s32 i = 0; i < buffer->cursor.row; ++i)
        pos += buffer->line_lengths[i] + 1;
    return pos;
}

static void render_batch_glyphs(Font_Render_Context* render_ctx, s32 count)
{
    glUniformMatrix4fv(glGetUniformLocation(render_ctx->program, "u_transforms"), count, GL_FALSE, (f32*)render_ctx->transforms);
    glUniform1uiv(glGetUniformLocation(render_ctx->program, "u_charmap"), count, render_ctx->charmap);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
}

static void render_buffer(Ted_Context* ctx, s16 buffer_idx)
{
    assert(buffer_idx < ctx->buffer_count);
    
    auto* buffer = ctx->buffers + buffer_idx;
    const auto* display_buffer = &buffer->display_buffer;
    const auto* atlas = active_atlas(ctx);

    // Render buffer contents.
    glUseProgram(ctx->font_render_ctx->program);
    glBindVertexArray(ctx->font_render_ctx->vao);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->font_render_ctx->vbo);
    glBindTexture(GL_TEXTURE_2D_ARRAY, atlas->texture_array);
    
    glActiveTexture(GL_TEXTURE0);
    glUniform3f(glGetUniformLocation(ctx->font_render_ctx->program, "u_text_color"), ctx->text_color.r, ctx->text_color.g, ctx->text_color.b);
    
    const s32 buffer_data_size = data_size(&buffer->display_buffer);
    const s32 prefix_size = prefix_data_size(&buffer->display_buffer);
    
    s16 work_idx = 0;
    s32 x = buffer->x;
    s32 y = buffer->y;

    for (s32 i = 0; i < buffer_data_size; ++i)
    {
        if (y < 0) break;

        const char c = char_at(&buffer->display_buffer, i);

        // @Cleanup: super straightforward text culling,
        // don't like it, but it gets the job done for now, refactor later.
        if (y > ctx->window_h)
        {
            if (c == '\n') y -= atlas->line_height;
            continue;
        }

        if (c == '\n')
        {
            x = buffer->x;
            y -= atlas->line_height;
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
        
        mat4* transform = ctx->font_render_ctx->transforms + work_idx;
        identity(transform);
        translate(transform, vec3{gx, gy, 0.0f});
        scale(transform, vec3{gw, gh, 0.0f});

        ctx->font_render_ctx->charmap[work_idx] = ci;

        if (++work_idx >= FONT_RENDER_BATCH_SIZE)
        {
            render_batch_glyphs(ctx->font_render_ctx, work_idx);
            work_idx = 0;
        }

        x += metric->advance_width;
    }
    
    if (work_idx > 0) render_batch_glyphs(ctx->font_render_ctx, work_idx);
    
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Render simple cursor.
    const auto* cursor = &buffer->cursor;

    const s32 line_start_pos = line_start_pointer_pos(buffer);
    const s32 width_px = line_width_px_till_pointer(atlas, display_buffer, line_start_pos);
    
    const f32 cursor_x = width_px + 4.0f;
    const f32 cursor_y = (f32)(ctx->window_h - atlas->line_height) - cursor->row * atlas->line_height;

    identity(&buffer->cursor.transform);
    translate(&buffer->cursor.transform, vec3{cursor_x, cursor_y, 0.0f});
    scale(&buffer->cursor.transform, vec3{2.0f, (f32)atlas->line_height, 0.0f});

    glUseProgram(ctx->cursor_render_ctx->program);
    glBindVertexArray(ctx->cursor_render_ctx->vao);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->cursor_render_ctx->vbo);

    glUniform3f(glGetUniformLocation(ctx->cursor_render_ctx->program, "u_text_color"), ctx->text_color.r, ctx->text_color.g, ctx->text_color.b);
    glUniformMatrix4fv(glGetUniformLocation(ctx->cursor_render_ctx->program, "u_transform"), 1, GL_FALSE, (f32*)&buffer->cursor.transform);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

static s32 vert_offset_from_baseline(const Font* font, const Font_Atlas* atlas)
{
    return (s32)((font->ascent + font->line_gap) * atlas->px_h_scale);
}

void update_frame(Ted_Context* ctx)
{
    // @Cleanup: move to context or smth.
    static f32 prev_time = (f32)glfwGetTime();

    const auto* atlas = active_atlas(ctx);
    // @Cleanup: calculate only on window resize?
    ctx->buffer_min_y = ctx->window_h - vert_offset_from_baseline(ctx->font, atlas);

    // @Todo: update all opened buffers (feature to come).
    auto* buffer = active_buffer(ctx);
    auto* display_buffer = &buffer->display_buffer;
    
    buffer->min_x = -ctx->window_w; // @Todo: should be equal to longest line length
    buffer->max_y = ctx->buffer_min_y + (buffer->last_line_idx * atlas->line_height);
    buffer->x = clamp(buffer->x, buffer->min_x, ctx->buffer_max_x);
    buffer->y = clamp(buffer->y, ctx->buffer_min_y, buffer->max_y);
    
    glClearColor(ctx->bg_color.r, ctx->bg_color.g, ctx->bg_color.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    render_buffer(ctx, ctx->active_buffer_idx);
    
#if TED_DEBUG
    static char debug_str[512];
    s32 debug_str_size = sprintf(debug_str, "%.2fms %.ffps", ctx->dt * 1000.0f, 1 / ctx->dt);

    f32 x = ctx->window_w - ctx->debug_atlas->line_height * debug_str_size * 0.5f;
    f32 y = (f32)(ctx->window_h - ctx->debug_atlas->line_height);
    render_text(ctx->font_render_ctx, ctx->debug_atlas, debug_str, debug_str_size, 1.0f, x, y, 1.0f, 1.0f, 1.0f);

    debug_str_size = sprintf(debug_str, "pointer_pos=%d\ncursor=(%d, %d | %c)\nend=%d\ngap_start=%d\ngap_end=%d\nxy=(%d, %d)\nmin_xy=(%d, %d)\nmax_xy=(%d, %d)\nlast_line_idx=%d\nfont_size=%d",
                             pointer_pos(&buffer->display_buffer),
                             buffer->cursor.row, buffer->cursor.col, char_at_pointer(display_buffer),
                             total_data_size(display_buffer),
                             prefix_data_size(display_buffer),
                             (s32)(buffer->display_buffer.gap_end - buffer->display_buffer.start),
                             buffer->x, buffer->y, buffer->min_x, ctx->buffer_min_y, ctx->buffer_max_x, buffer->max_y, buffer->last_line_idx, atlas->font_size);

    x = ctx->window_w - ctx->debug_atlas->font_size * 12.0f;
    y -= ctx->debug_atlas->line_height;
    render_text(ctx->font_render_ctx, ctx->debug_atlas, debug_str, debug_str_size, 1.0f, x, y, 1.0f, 1.0f, 1.0f);
#endif
    
    glfwSwapBuffers(ctx->window);
    glfwPollEvents();

    const f32 time = (f32)glfwGetTime();
    ctx->dt = time - prev_time;
    prev_time = time;
}

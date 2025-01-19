#include "pch.h"
#include "ted.h"
#include "file.h"
#include "font.h"
#include "arena.h"
#include "gap_buffer.h"
#include <stdio.h>
#include <math.h>
#include <glad/glad.h>
#include <glfw/glfw3.h>

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

    if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
        move_cursor(buffer->display_buffer, -1);
        
    if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
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
    //printf("x, y scroll: %.2f, %.2f\n", xoffset, yoffset);
    
    auto* ctx = (Ted_Context*)glfwGetWindowUserPointer(window);
    auto* atlas = active_atlas(ctx);
    auto* buffer = active_buffer(ctx);

    // @Todo: correct scroll for buffers that do not fit into window (hor and/or vert).
    if (yoffset)
    {
        const f32 max_y = (f32)(ctx->window_h - atlas->font_size);
        buffer->y -= (f32)yoffset * atlas->font_size;
        buffer->y = clamp(buffer->y, buffer->min_y, max_y);
    }
    
    if (xoffset)
    {
        buffer->x -= (f32)xoffset * atlas->font_size;
        buffer->x = clamp(buffer->x, ctx->buffer_min_x, buffer->max_x);
    }
}

static void drop_callback(GLFWwindow* window, s32 count, const char** paths)
{
    auto* ctx = (Ted_Context*)glfwGetWindowUserPointer(window);
    for (s32 i = 0; i < count; ++i)
    {
         // @Cleanup: maybe its a bit annoying to write such procedure every time
         // you want to open a new file and we should tweak api to have just one
         // function that opens file in new buffer.
         const s16 new_active_buffer = create_buffer(ctx);
         set_active_buffer(ctx, new_active_buffer);
         load_file_contents(ctx, new_active_buffer, paths[i]);
    }
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

void init_ted_context(Arena* arena, Ted_Context* ctx)
{
    *ctx = {0};
    ctx->arena = arena;
    ctx->font = push_struct(arena, Font);
    ctx->atlases = push_array(arena, TED_MAX_ATLASES, Font_Atlas);
    ctx->render_ctx = push_struct(arena, Font_Render_Context);
    ctx->buffers = push_array(arena, TED_MAX_BUFFERS, Ted_Buffer);
    ctx->bg_color = vec3{2.0f / 255.0f, 26.0f / 255.0f, 25.0f / 255.0f};
    ctx->text_color = vec3{255.0f / 255.0f, 220.0f / 255.0f, 194.0f / 255.0f};
    ctx->buffer_min_x = 4.0f; // @Todo: make it customizable constant.
}

void destroy(Ted_Context* ctx)
{    
    // @Cleanup: these frees should not be here after gap buffer will use memory arena.
    for (s16 i = 0; i < ctx->buffer_count; ++i)
        free(ctx->buffers[i].display_buffer);

    clear(ctx->arena);
    glfwTerminate();
}

bool alive(Ted_Context* ctx)
{
    assert(ctx->window);
    return !glfwWindowShouldClose(ctx->window);
}

void create_window(Ted_Context* ctx, s16 win_w, s16 win_h, const char* name)
{
    assert(!ctx->window);
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    ctx->window = glfwCreateWindow(win_w, win_h, name, null, null);
    if (!ctx->window)
    {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return;
    }

    ctx->window_w = win_w;
    ctx->window_h = win_h;

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
    init_font(ctx->arena, ctx->font, path);
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

    init_font_render_context(ctx->arena, ctx->render_ctx, ctx->window_w, ctx->window_h);
}

void bake_font(Ted_Context* ctx, u32 start_charcode, u32 end_charcode, s16 min_font_size, s16 max_font_size, s16 font_size_stride)
{
    s16 i = 0;
    for (s16 font_size = min_font_size; font_size <= max_font_size; font_size += font_size_stride, ++i)
    {
        if (i >= TED_MAX_ATLASES) break;
        bake_font_atlas(ctx->arena, ctx->font, ctx->atlases + i, start_charcode, end_charcode, font_size);
    }

    ctx->atlas_count = i;
    ctx->active_atlas_idx = i / 2;
}

s16 create_buffer(Ted_Context* ctx)
{
    if (ctx->buffer_count > TED_MAX_BUFFERS) return -1;

    auto* atlas = active_atlas(ctx);
    auto* buffer = ctx->buffers + ctx->buffer_count;
    buffer->arena = subarena(ctx->arena, TED_MAX_BUFFER_SIZE);
    buffer->display_buffer = push_struct(ctx->arena, Gap_Buffer);
    buffer->x = ctx->buffer_min_x;
    buffer->y = (f32)(ctx->window_h - atlas->font_size);

    // @Cleanup: pass arena or smth.
    init(buffer->display_buffer, 32);
    
    return ctx->buffer_count++;
}

void load_file_contents(Ted_Context* ctx, s16 buffer_idx, const char* path)
{
    if (buffer_idx >= ctx->buffer_count) return;

    auto* buffer = ctx->buffers + buffer_idx;
    clear(&buffer->arena);
    
    s32 size = 0;
    u8* data = read_entire_file(&buffer->arena, path, &size);

    // Handle non-ascii?
    push_str(buffer->display_buffer, (char*)data, size);
}

void close_buffer(Ted_Context* ctx, s16 buffer_idx)
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
}

void open_next_buffer(Ted_Context* ctx)
{
    ctx->active_buffer_idx++;
    if (ctx->active_buffer_idx >= ctx->buffer_count)
        ctx->active_buffer_idx = 0;
}

void open_prev_buffer(Ted_Context* ctx)
{
    ctx->active_buffer_idx--;
    if (ctx->active_buffer_idx < 0)
        ctx->active_buffer_idx = ctx->buffer_count - 1;
}

void increase_font_size(Ted_Context* ctx)
{
    const s16 old_font_size = active_atlas(ctx)->font_size;
    
    ctx->active_atlas_idx++;
    if (ctx->active_atlas_idx >= ctx->atlas_count)
        ctx->active_atlas_idx = ctx->atlas_count - 1;

    const auto* atlas = active_atlas(ctx);
    auto* buffer = active_buffer(ctx);
    // @Cleanup: fix buffer y pos after font size change.
    //buffer->y += (f32)(atlas->font_size - old_font_size);
}

void decrease_font_size(Ted_Context* ctx)
{
    const s16 old_font_size = active_atlas(ctx)->font_size;
    
    ctx->active_atlas_idx--;
    if (ctx->active_atlas_idx < 0)
        ctx->active_atlas_idx = 0;

    const auto* atlas = active_atlas(ctx);
    auto* buffer = active_buffer(ctx);
    //buffer->y += (f32)(atlas->font_size - old_font_size);
}

void update_frame(Ted_Context* ctx)
{
    glClearColor(ctx->bg_color.r, ctx->bg_color.g, ctx->bg_color.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const auto* buffer = active_buffer(ctx);
    const auto* atlas = active_atlas(ctx);

    // @Cleanup: we probably want own render_text implementation.
    render_text(ctx->render_ctx, atlas, buffer->display_buffer, 1.0f, buffer->x, buffer->y, ctx->text_color.r, ctx->text_color.g, ctx->text_color.b);

#if TED_DRAW_DEBUG_INFO
    atlas = ctx->atlases + 2; // use smaller atlas for debug info
    static char debug_info_str[512];
    const s32 debug_info_str_size = sprintf(debug_info_str, "cursor_pos=%lld, end=%lld, gap_start=%lld, gap_end=%lld, buffer_y=%.2f\n", cursor_pos(buffer->display_buffer), total_data_size(buffer->display_buffer), prefix_data_size(buffer->display_buffer), buffer->display_buffer->gap_end - buffer->display_buffer->start, buffer->y);
    render_text(ctx->render_ctx, atlas, debug_info_str, debug_info_str_size, 1.0f, atlas->font_size * 0.5f, atlas->font_size * 0.5f, 1.0f, 1.0f, 1.0f);
#endif
    
    glfwSwapBuffers(ctx->window);
    glfwPollEvents();
}

#pragma once

#include "arena.h"
#include "vector.h"

struct Font;
struct Font_Atlas;
struct Font_Render_Context;
struct Gap_Buffer;
struct GLFWwindow;

#define TED_DRAW_DEBUG_INFO 1

inline constexpr s32 TED_MAX_BUFFERS = 32;
inline constexpr s32 TED_MAX_BUFFER_SIZE = KB(256);
inline constexpr s32 TED_MAX_ATLASES = 64;

struct Ted_Buffer
{
    Arena arena;
    Gap_Buffer* display_buffer;
    f32 x;
    f32 y;
    f32 max_x; // @Todo: depends on longest line size?
    f32 min_y; // @Todo: depends on line count?
};

struct Ted_Context
{
    Arena* arena;
    GLFWwindow* window;
    Font* font;
    Font_Atlas* atlases;
    Font_Render_Context* render_ctx;
    Ted_Buffer* buffers;
    s16 atlas_count;
    s16 active_atlas_idx;
    s16 buffer_count;
    s16 active_buffer_idx;
    s16 window_w;
    s16 window_h;
    vec3 bg_color;
    vec3 text_color;
    f32 buffer_min_x;
};

Ted_Buffer* active_buffer(Ted_Context* ctx);
Font_Atlas* active_atlas(Ted_Context* ctx);

void init_ted_context(Arena* arena, Ted_Context* ctx);
void destroy(Ted_Context* ctx);
bool alive(Ted_Context* ctx);
void create_window(Ted_Context* ctx, s16 win_w, s16 win_h, const char* name);
void load_font(Ted_Context* ctx, const char* path);
void init_render_context(Ted_Context* ctx);
void bake_font(Ted_Context* ctx, u32 start_charcode, u32 end_charcode, s16 min_font_size, s16 max_font_size, s16 font_size_stride);
s16 create_buffer(Ted_Context* ctx);
void load_file_contents(Ted_Context* ctx, s16 buffer_idx, const char* path);
void close_buffer(Ted_Context* ctx, s16 buffer_idx);
void set_active_buffer(Ted_Context* ctx, s16 buffer_idx);
void open_next_buffer(Ted_Context* ctx);
void open_prev_buffer(Ted_Context* ctx);
void increase_font_size(Ted_Context* ctx);
void decrease_font_size(Ted_Context* ctx);
void update_frame(Ted_Context* ctx);

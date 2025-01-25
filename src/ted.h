#pragma once

#include "file.h"
#include "arena.h"
#include "vector.h"
#include "gap_buffer.h"

struct Font;
struct Font_Atlas;
struct Font_Render_Context;
struct Gap_Buffer;
struct GLFWwindow;

#define TED_DEBUG 1

inline constexpr s32 TED_MAX_BUFFERS = 64;
inline constexpr s32 TED_MAX_ATLASES = 64;
inline constexpr s32 TED_MAX_LINE_COUNT = 32 * 1024;
inline constexpr s32 TED_MAX_FILE_SIZE = KB(256);
inline constexpr s32 TED_MAX_FILE_NAME_SIZE = 256;
inline constexpr s32 TED_MAX_BUFFER_SIZE = TED_MAX_FILE_NAME_SIZE + TED_MAX_FILE_SIZE + (TED_MAX_LINE_COUNT * sizeof(s32));

// @Todo
struct Ted_Cursor
{
    s32 x;
    s32 y;
};

struct Ted_Buffer
{
    Arena arena; // is meant for buffer metadata and contents
    Gap_Buffer display_buffer;
    char* path; // path used to load file contents
    s32* new_line_offsets;
    s32 new_line_count;
    s32 x;
    s32 y;
    s32 min_x; // @Todo: depends on longest line size?
    s32 max_y;
};

struct Ted_Context
{
    Arena arena;
    GLFWwindow* window;
    Font* font;
    Font_Atlas* atlases;
    Font_Render_Context* render_ctx;
    Ted_Buffer* buffers;
    vec3 bg_color;
    vec3 text_color;
    f32 dt;
    s32 buffer_max_x;
    s32 buffer_min_y;
    s16 atlas_count;
    s16 active_atlas_idx;
    s16 buffer_count;
    s16 active_buffer_idx;
    s16 window_w;
    s16 window_h;
    
#if TED_DEBUG
    Font_Atlas* debug_atlas;
#endif
};

Ted_Buffer* active_buffer(Ted_Context* ctx);
Font_Atlas* active_atlas(Ted_Context* ctx);

void init_ted_context(Ted_Context* ctx, void* memory, u32 size);
void destroy(Ted_Context* ctx);
bool alive(Ted_Context* ctx);
void create_window(Ted_Context* ctx, s16 w, s16 h, s16 x, s16 y);
void load_font(Ted_Context* ctx, const char* path);
void init_render_context(Ted_Context* ctx);
void bake_font(Ted_Context* ctx, u32 start_charcode, u32 end_charcode, s16 min_font_size, s16 max_font_size, s16 font_size_stride);
s16 create_buffer(Ted_Context* ctx);
void load_file_contents(Ted_Context* ctx, s16 buffer_idx, const char* path);
void kill_buffer(Ted_Context* ctx, s16 buffer_idx);
void set_active_buffer(Ted_Context* ctx, s16 buffer_idx);
void open_next_buffer(Ted_Context* ctx);
void open_prev_buffer(Ted_Context* ctx);
void increase_font_size(Ted_Context* ctx);
void decrease_font_size(Ted_Context* ctx);
void push_char(Ted_Context* ctx, s16 buffer_idx, char c);
void push_str(Ted_Context* ctx, s16 buffer_idx, const char* str, s32 size);
void delete_char(Ted_Context* ctx, s16 buffer_idx);
void delete_char_overwrite(Ted_Context* ctx, s16 buffer_idx);
void update_frame(Ted_Context* ctx);

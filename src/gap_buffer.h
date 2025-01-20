#pragma once

// @Cleanup: use memory arenas, malloc is used as temp solution.

inline constexpr s64 GAP_EXPAND_SIZE = 16;
 
struct Gap_Buffer
{
    char* start;
    char* end;
    char* cursor;
    char* gap_start;
    char* gap_end;
};

s64 cursor_pos(const Gap_Buffer* buffer);
s64 total_data_size(const Gap_Buffer* buffer);
s64 gap_data_size(const Gap_Buffer* buffer);
s64 data_size(const Gap_Buffer* buffer);
s64 prefix_data_size(const Gap_Buffer* buffer);
s64 suffix_data_size(const Gap_Buffer* buffer);

void init_gap_buffer(Gap_Buffer* buffer, s64 size);
void free(Gap_Buffer* buffer);
void expand(Gap_Buffer* buffer, s64 extra_size = 0);
void set_cursor(Gap_Buffer* buffer, s64 pos);
void move_cursor(Gap_Buffer* buffer, s64 delta);
void push_char(Gap_Buffer* buffer, char c);
void push_str(Gap_Buffer* buffer, const char* str, s64 size);
void delete_char(Gap_Buffer* buffer);
void delete_char_overwrite(Gap_Buffer* buffer);

// @Cleanup: not sure about these.
s64 fill_utf8(const Gap_Buffer* buffer, char* data);
s64 fill_utf32(const Gap_Buffer* buffer, u32* data);
s64 fill_debug_utf32(const Gap_Buffer* buffer, u32* data);

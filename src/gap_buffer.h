#pragma once

// @Cleanup: use memory arenas, malloc is used as temp solution. ???

inline constexpr char INVALID_CHAR = 0;
inline constexpr s32 GAP_EXPAND_SIZE = 16;

struct Gap_Buffer
{
    char* start;
    char* end;
    char* pointer;
    char* gap_start;
    char* gap_end;
};

s32 pointer_pos(const Gap_Buffer* buffer);
s32 total_data_size(const Gap_Buffer* buffer);
s32 gap_data_size(const Gap_Buffer* buffer);
s32 data_size(const Gap_Buffer* buffer);
s32 prefix_data_size(const Gap_Buffer* buffer);
s32 suffix_data_size(const Gap_Buffer* buffer);
char char_at(const Gap_Buffer* buffer, s32 pos);
char char_at_pointer(const Gap_Buffer* buffer); // be care of pointer == gap_start
char char_before_pointer(const Gap_Buffer* buffer);

void init_gap_buffer(Gap_Buffer* buffer, s32 size);
void free(Gap_Buffer* buffer);
void expand(Gap_Buffer* buffer, s32 extra_size = 0);
void set_pointer(Gap_Buffer* buffer, s32 pos);
void move_pointer(Gap_Buffer* buffer, s32 delta);
void push_char(Gap_Buffer* buffer, char c);
void push_str(Gap_Buffer* buffer, const char* str, s32 size);
char delete_char(Gap_Buffer* buffer);
char delete_char_overwrite(Gap_Buffer* buffer);

void move_gap_to_pointer(Gap_Buffer* buffer);

// @Cleanup: not sure about these.
s32 fill_utf8(const Gap_Buffer* buffer, char* data);
s32 fill_utf32(const Gap_Buffer* buffer, u32* data);
s32 fill_debug_utf32(const Gap_Buffer* buffer, u32* data);

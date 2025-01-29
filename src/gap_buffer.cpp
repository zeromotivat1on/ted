#include "pch.h"
#include "gap_buffer.h"
#include "arena.h"
#include <stdio.h>
#include <malloc.h>

s32 pointer_pos(const Gap_Buffer* buffer)
{
    if (buffer->pointer <= buffer->gap_start)
        return (s32)(buffer->pointer - buffer->start);
    else
        return (s32)(buffer->pointer - buffer->start - gap_data_size(buffer));
}

s32 total_data_size(const Gap_Buffer* buffer)
{
    return (s32)(buffer->end - buffer->start);
}

s32 gap_data_size(const Gap_Buffer* buffer)
{
    return (s32)(buffer->gap_end - buffer->gap_start);
}

s32 data_size(const Gap_Buffer* buffer)
{
    return total_data_size(buffer) - gap_data_size(buffer);
}

s32 prefix_data_size(const Gap_Buffer* buffer)
{
    return (s32)(buffer->gap_start - buffer->start);
}

s32 suffix_data_size(const Gap_Buffer* buffer)
{
    return (s32)(buffer->end - buffer->gap_end);
}

char char_at(const Gap_Buffer* buffer, s32 pos)
{
    if (pos == data_size(buffer)) return INVALID_CHAR; // nochekin
    
    assert(pos >= 0);
    assert(pos < data_size(buffer));

    const auto* p = buffer->start + pos;
    if (p >= buffer->gap_start) p += gap_data_size(buffer);
    return *p;
}

char char_at_pointer(const Gap_Buffer* buffer)
{
    return char_at(buffer, pointer_pos(buffer));
}

char char_before_pointer(const Gap_Buffer* buffer)
{
    const s32 pos = pointer_pos(buffer);
    return char_at(buffer, max(0, pos - 1));
}

void init_gap_buffer(Gap_Buffer* buffer, s32 size)
{
    const s32 total_size = size + GAP_EXPAND_SIZE;
    buffer->start = (char*)malloc(total_size);
    buffer->end = buffer->start + total_size;
    buffer->pointer = buffer->start;
    buffer->gap_start = buffer->start;
    buffer->gap_end = buffer->start + total_size;
}

void free(Gap_Buffer* buffer)
{
    assert(buffer->start);
    free(buffer->start);
    *buffer = {0};
}

void expand(Gap_Buffer* buffer, s32 extra_size)
{
    const char* old_start = buffer->start;
    const s32 add_size = extra_size + GAP_EXPAND_SIZE;
    const s32 new_size = total_data_size(buffer) + add_size;
    buffer->start = (char*)realloc(buffer->start, new_size);

    const s32 offset = (s32)(buffer->start - old_start);
    buffer->end += offset;
    buffer->pointer += offset;
    buffer->gap_start += offset;
    buffer->gap_end += offset;

    memmove(buffer->gap_end + add_size, buffer->gap_end, buffer->end - buffer->gap_end);
    buffer->end += add_size;
    buffer->gap_end += add_size;
}

void set_pointer(Gap_Buffer* buffer, s32 pos)
{
    assert(pos >= 0);
    assert(pos <= data_size(buffer));
    
    buffer->pointer = buffer->start + pos;

    if (buffer->pointer > buffer->gap_start)
        buffer->pointer += gap_data_size(buffer);
}

void move_pointer(Gap_Buffer* buffer, s32 delta)
{
    set_pointer(buffer, pointer_pos(buffer) + delta);
}

void push_char(Gap_Buffer* buffer, char c)
{
    move_gap_to_pointer(buffer);
    if (buffer->gap_start == buffer->gap_end) expand(buffer);
    *buffer->gap_start++ = c;
    buffer->pointer = buffer->gap_start;
}

void push_str(Gap_Buffer* buffer, const char* str, s32 size)
{
    move_gap_to_pointer(buffer);
    if (size > gap_data_size(buffer)) expand(buffer, size);

    for (s32 i = 0; i < size; ++i)
        *buffer->gap_start++ = str[i];
    
    buffer->pointer = buffer->gap_start;
}

char delete_char(Gap_Buffer* buffer)
{
    if (buffer->pointer == buffer->start) return INVALID_CHAR;
    move_gap_to_pointer(buffer);
    const char c = *(--buffer->gap_start);
    buffer->pointer = buffer->gap_start;
    return c;
}

char delete_char_overwrite(Gap_Buffer* buffer)
{
    move_gap_to_pointer(buffer);
    if (buffer->gap_end == buffer->end) return INVALID_CHAR;
    return *buffer->gap_end++;
}

s32 fill_utf8(const Gap_Buffer* buffer, char* data)
{
    const s32 prefix_size = prefix_data_size(buffer);
    const s32 suffix_size = suffix_data_size(buffer);
    memcpy(data, buffer->start, prefix_size);
    memcpy(data + prefix_size, buffer->gap_end, suffix_size);

    const s32 size = prefix_size + suffix_size;
    data[size] = '\0';
    return size;
}

s32 fill_utf32(const Gap_Buffer* buffer, u32* data)
{          
    const s32 prefix_size = prefix_data_size(buffer);
    for (s32 i = 0; i < prefix_size; ++i)
        data[i] = (u32)buffer->start[i];
        
    const s32 suffix_size = suffix_data_size(buffer);
    for (s32 i = 0; i < suffix_size; ++i)
        data[i + prefix_size] = (u32)buffer->gap_end[i];

    const s32 size = prefix_size + suffix_size;
    data[size] = '\0';
    return size;
}

s32 fill_debug_utf32(const Gap_Buffer* buffer, u32* data)
{
    const char* c = buffer->start;

    s32 i = 0;
    while (c < buffer->end)
    {
        if (c >= buffer->gap_start && c < buffer->gap_end) data[i] = '_';
        else data[i] = *c;
        c++;
        i++;
    }
    
    data[i] = '\0';
    return i - 1;
}

void move_gap_to_pointer(Gap_Buffer* buffer)
{
    if (buffer->pointer == buffer->gap_start) return;

    if (buffer->pointer < buffer->gap_start)
    {
        const s32 move_size = (s32)(buffer->gap_start - buffer->pointer);
        memmove(buffer->pointer + gap_data_size(buffer), buffer->pointer, move_size);
        buffer->gap_end -= move_size;
        buffer->gap_start = buffer->pointer;
    }
    else
    {
        const s32 move_size = (s32)(buffer->pointer - buffer->gap_end);
        memmove(buffer->gap_start, buffer->gap_end, move_size);
        buffer->gap_start += move_size;
        buffer->gap_end = buffer->pointer;
        buffer->pointer = buffer->gap_start;
    }
}

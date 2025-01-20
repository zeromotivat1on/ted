#include "pch.h"
#include "gap_buffer.h"
#include "arena.h"
#include <stdio.h>
#include <malloc.h>

static char* convert_user_to_gap_space(const Gap_Buffer* buffer, char* c)
{
    if (c <= buffer->gap_start) return c;
    if (c < buffer->gap_end) return c + (buffer->gap_end - buffer->gap_start);
    return null;
}

static void move_gap_to_cursor(Gap_Buffer* buffer)
{
    char* gsp_cursor = convert_user_to_gap_space(buffer, buffer->cursor);
    assert(gsp_cursor); // unexpected cursor position

    if (gsp_cursor == buffer->gap_start) return;

    if (gsp_cursor < buffer->gap_start)
    {
        const s64 move_size = buffer->gap_start - gsp_cursor;
        memmove(gsp_cursor + gap_data_size(buffer), gsp_cursor, move_size);
        buffer->gap_end -= move_size;
        buffer->gap_start = gsp_cursor;
    }
    else
    {
        const s64 move_size = gsp_cursor - buffer->gap_end;
        memmove(buffer->gap_start, buffer->gap_end, move_size);
        buffer->gap_start += move_size;
        buffer->gap_end = gsp_cursor;
        buffer->cursor = buffer->gap_start;
    }
}

s64 cursor_pos(const Gap_Buffer* buffer)
{
    return buffer->cursor - buffer->start;
}

s64 total_data_size(const Gap_Buffer* buffer)
{
    return buffer->end - buffer->start;
}

s64 gap_data_size(const Gap_Buffer* buffer)
{
    return buffer->gap_end - buffer->gap_start;    
}

s64 data_size(const Gap_Buffer* buffer)
{
    return total_data_size(buffer) - gap_data_size(buffer);
}

s64 prefix_data_size(const Gap_Buffer* buffer)
{
    return buffer->gap_start - buffer->start;
}

s64 suffix_data_size(const Gap_Buffer* buffer)
{
    return buffer->end - buffer->gap_end;
}

void init_gap_buffer(Gap_Buffer* buffer, s64 size)
{
    const s64 total_size = size + GAP_EXPAND_SIZE;
    buffer->start = (char*)malloc(total_size);
    buffer->end = buffer->start + total_size;
    buffer->cursor = buffer->start;
    buffer->gap_start = buffer->start;
    buffer->gap_end = buffer->start + total_size;
}

void free(Gap_Buffer* buffer)
{
    assert(buffer->start);
    free(buffer->start);
    *buffer = {0};
}

void expand(Gap_Buffer* buffer, s64 extra_size)
{
    const char* old_start = buffer->start;
    const s64 add_size = extra_size + GAP_EXPAND_SIZE;
    const s64 new_size = total_data_size(buffer) + add_size;
    buffer->start = (char*)realloc(buffer->start, new_size);

    const s64 offset = buffer->start - old_start;
    buffer->end += offset;
    buffer->cursor += offset;
    buffer->gap_start += offset;
    buffer->gap_end += offset;

    memmove(buffer->gap_end + add_size, buffer->gap_end, buffer->end - buffer->gap_end);
    buffer->end += add_size;
    buffer->gap_end += add_size;
}

void set_cursor(Gap_Buffer* buffer, s64 pos)
{
    const s64 max_pos = data_size(buffer);
    if (pos < 0) pos = 0;
    else if (pos > max_pos) pos = max_pos;
    buffer->cursor = buffer->start + pos;
}

void move_cursor(Gap_Buffer* buffer, s64 delta)
{
    set_cursor(buffer, cursor_pos(buffer) + delta);
}

void push_char(Gap_Buffer* buffer, char c)
{
    move_gap_to_cursor(buffer);
    if (buffer->gap_start == buffer->gap_end) expand(buffer);
    *buffer->gap_start++ = c;
    buffer->cursor = buffer->gap_start;
}

void push_str(Gap_Buffer* buffer, const char* str, s64 size)
{
    move_gap_to_cursor(buffer);
    if (size > gap_data_size(buffer)) expand(buffer, size);

    for (s64 i = 0; i < size; ++i)
        *buffer->gap_start++ = str[i];
    
    buffer->cursor = buffer->gap_start;
}

void delete_char(Gap_Buffer* buffer)
{
    if (buffer->cursor == buffer->start) return;
    move_gap_to_cursor(buffer);
    buffer->gap_start--;
    buffer->cursor = buffer->gap_start;
}

void delete_char_overwrite(Gap_Buffer* buffer)
{
    move_gap_to_cursor(buffer);
    if (buffer->gap_end == buffer->end) return;
    buffer->gap_end++;
}

s64 fill_utf8(const Gap_Buffer* buffer, char* data)
{
    const s64 prefix_size = prefix_data_size(buffer);
    const s64 suffix_size = suffix_data_size(buffer);
    memcpy(data, buffer->start, prefix_size);
    memcpy(data + prefix_size, buffer->gap_end, suffix_size);

    const s64 size = prefix_size + suffix_size;
    data[size] = '\0';
    return size;
}

s64 fill_utf32(const Gap_Buffer* buffer, u32* data)
{          
    const s64 prefix_size = prefix_data_size(buffer);
    for (s64 i = 0; i < prefix_size; ++i)
        data[i] = (u32)buffer->start[i];
        
    const s64 suffix_size = suffix_data_size(buffer);
    for (s64 i = 0; i < suffix_size; ++i)
        data[i + prefix_size] = (u32)buffer->gap_end[i];

    const s64 size = prefix_size + suffix_size;
    data[size] = '\0';
    return size;
}

s64 fill_debug_utf32(const Gap_Buffer* buffer, u32* data)
{
    const char* c = buffer->start;

    s64 i = 0;
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

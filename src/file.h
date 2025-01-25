#pragma once

struct Arena;

u8* read_entire_file(Arena* arena, const char* path, s32* size_pushed = null);
void overwrite_file(const char* path, const u8* data, s32 size);

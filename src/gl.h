#pragma once

#include "vector.h"
#include "matrix.h"

struct Arena;

u32 gl_create_program(const char* vs_src, const char* fs_src);
u32 gl_load_program(Arena* arena, const char* vs_path, const char* fs_path);

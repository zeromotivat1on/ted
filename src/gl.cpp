#include "pch.h"
#include "gl.h"
#include "file.h"
#include "arena.h"
#include <stdio.h>
#include <glad/glad.h>

u32 gl_create_program(const char* vs_src, const char* fs_src)
{
    static char info_log[1024];

    assert(vs_src);
    assert(fs_src);
    
    u32 vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_src, null);
    glCompileShader(vs);

    s32 success;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vs, sizeof(info_log), null, info_log);
        printf("Failed to compile vertex shader: %s\n", info_log);
        return -1;
    }
    
    u32 fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_src, null);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fs, sizeof(info_log), null, info_log);
        printf("Failed to compile fragment shader: %s\n", info_log);
        return -1;
    }

    u32 program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, sizeof(info_log), null, info_log);
        printf("Failed to link shader program: %s\n", info_log);
        return -1;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

u32 gl_load_program(Arena* arena, const char* vs_path, const char* fs_path)
{
    s32 vs_size = 0;
    const char* vs_src = (char*)read_entire_file(arena, vs_path, &vs_size);
    
    s32 fs_size = 0;
    const char* fs_src = (char*)read_entire_file(arena, fs_path, &fs_size);

    const u32 program = gl_create_program(vs_src, fs_src);

    pop(arena, fs_size);
    pop(arena, vs_size);

    return program;
}

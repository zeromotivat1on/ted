#include "pch.h"
#include "font.h"
#include "arena.h"
#include "matrix.h"
#include <stdio.h>
#include <malloc.h>
#include <glad/glad.h>
#include <glfw/glfw3.h>

u8* read_entire_file(Arena* arena, const char* path, s32* size_read)
{
    if (FILE* file = fopen(path, "rb"))
    {
        fseek(file, 0, SEEK_END);
        const s32 size = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (size_read)
            *size_read = size;

        u8* file_data = push(arena, size);
        const u64 read_amount = fread(file_data, size, 1, file);
        file_data[size] = '\0';

        if (read_amount)
        {
            fclose(file);
            return file_data;
        }
        
        pop(arena, size);
        fclose(file);
    }

    return nullptr;
}

struct GL_Glyph
{
    Glyph_Slot* slot;
    f32* vertices; // xy pairs
    u32 program;
    u32 vbo;
    u32 vao;
};

void gl_init_glyph(Arena* arena, u32 program, Glyph_Slot* slot, GL_Glyph* glyph)
{
    glyph->slot = slot;
    glyph->program = program;

    const u16 vertex_count = get_point_count(slot) * 2;
    glyph->vertices = push_array(arena, vertex_count, f32);
    for (u16 i = 0, coord_idx = 0; i < vertex_count; i += 2, ++coord_idx)
    {
        glyph->vertices[i + 0] = (f32)slot->x_coordinates[coord_idx];
        glyph->vertices[i + 1] = (f32)slot->y_coordinates[coord_idx];
	}
 
    glGenBuffers(1, &glyph->vbo);
    glGenVertexArrays(1, &glyph->vao);

    glBindVertexArray(glyph->vao);
    glBindBuffer(GL_ARRAY_BUFFER, glyph->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(f32), glyph->vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);  

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void gl_transform_glyph(GL_Glyph* glyph, f32 scale, f32 x_translate, f32 y_translate)
{
    glBindBuffer(GL_ARRAY_BUFFER, glyph->vbo);

    const u16 vertex_count = get_point_count(glyph->slot) * 2;
    for (u16 i = 0; i < vertex_count; i += 2)
    {   
        glyph->vertices[i + 0] = glyph->vertices[i + 0] * scale + x_translate;
        glyph->vertices[i + 1] = glyph->vertices[i + 1] * scale + y_translate;
	}

    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(f32), glyph->vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

u32 gl_create_program(const char* vertex_shader_src, const char* fragment_shader_src)
{
    u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_src, NULL);
    glCompileShader(vertex_shader);

    s32 success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        printf("Failed to compile vertex shader\n");
        return -1;
    }
    
    u32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_src, NULL);
    glCompileShader(fragment_shader);

    success;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        printf("Failed to compile fragment shader\n");
        return -1;
    }

    u32 shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);

    if (!success)
    {
        printf("Failed to link shader program\n");
        return -1;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

void draw_glyph(Glyph_Slot* glyph, u32 program, u32 vao, s32 draw_mode)
{
    glUseProgram(program);
    glBindVertexArray(vao);

    u16 start_idx = 0;
    u16 count = glyph->end_pts_of_countours[0] + 1;

    glDrawArrays(draw_mode, start_idx, count);
    start_idx += count;
    
    for (s16 i = 1; i < glyph->number_of_countours; ++i)
    {
        // @Todo: not sure if this will be correct for all glyphs.
        count = glyph->end_pts_of_countours[i] - glyph->end_pts_of_countours[i - 1];
        glDrawArrays(draw_mode, start_idx, count);
        start_idx += count;
    }
}

void char_callback(GLFWwindow* win, u32 character)
{
    printf("Window char (%c)\n", character);
}

void framebuffer_size_callback(GLFWwindow* window, s32 width, s32 height)
{
    glViewport(0, 0, width, height);
}

int main()
{
#if 0
    const char* font_path = "C:/Users/admin/Downloads/Envy_Code_R_PR7/Envy_Code_R.ttf";
#else
    const char* font_path = "C:/Users/admin/Downloads/consola.ttf";
#endif
    
    constexpr u64 k_app_arena_size = MB(4);
    Arena app_arena = create_arena(malloc(k_app_arena_size), k_app_arena_size);
    Arena font_arena = subarena(&app_arena, MB(1));

    Font_Face font_face;
    init_font_face(&font_arena, font_path, &font_face);
    
    const u16 units_per_em = font_face.dir->head.units_per_em;
    const u16 num_glyphs = font_face.dir->maxp.num_glyphs;
    printf("Font units_per_em (%u) num_glyphs (%u)\n", units_per_em, num_glyphs);

    print_font_directory(font_face.dir);
    print_cmap(&font_face.dir->cmap);
    print_format4(font_face.dir->f4);
    
    const bool loaded = load_char(&font_face, 'A');
    assert(loaded);
    print_glyph_data(font_face.glyph);
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    constexpr s32 k_window_width = 800;
    constexpr s32 k_window_height = 600;
    GLFWwindow* window = glfwCreateWindow(k_window_width, k_window_height, "ted", NULL, NULL);
    if (!window)
    {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCharCallback(window, char_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    const char* vertex_shader_src =
        "#version 460 core\n"
        "layout (location = 0) in vec2 vpos;\n"
        "uniform mat4 projection;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = projection * vec4(vpos.xy, 0.0f, 1.0f);\n"
        "}\0";
    
    const char* fragment_shader_src =
        "#version 460 core\n"
        "out vec4 ocolor;\n"
        "void main()\n"
        "{\n"
        "    ocolor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\0";
    
    const u32 shader_program = gl_create_program(vertex_shader_src, fragment_shader_src);

    mat4 projection = ortho(0.0f, k_window_width, 0.0f, k_window_height, -1.0f, 1.0f);
    glUseProgram(shader_program);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, (f32*)projection.rows);

    const u16 font_size = 128;
    const f32 font_scale = (f32)font_size / (f32)units_per_em;

    const f32 glyph_half_w = get_width(font_face.glyph) * font_scale * 0.5f;
    const f32 glyph_half_h = get_height(font_face.glyph) * font_scale * 0.5f;
    
    GL_Glyph glyph;
    gl_init_glyph(&app_arena, shader_program, font_face.glyph, &glyph);
    gl_transform_glyph(&glyph, font_scale, k_window_width / 2 - glyph_half_w, k_window_height / 2 - glyph_half_h);
    
    f32 dt = 0.0f;
    f32 frame_time = (f32)glfwGetTime();
    s32 glyph_draw_mode = GL_LINE_LOOP;
    
    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            glyph_draw_mode = GL_LINE_LOOP;

        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
            glyph_draw_mode = GL_POINTS;

        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
            glyph_draw_mode = GL_TRIANGLES;
                
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        draw_glyph(glyph.slot, glyph.program, glyph.vao, glyph_draw_mode);
        //glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        const f32 time = (f32)glfwGetTime();
        dt = time - frame_time;
        frame_time = time;
        
        //printf("dt (%.6fs)\n", dt);
    }

    //glDeleteVertexArrays(1, &vao);
    //glDeleteBuffers(1, &vbo);
    //glDeleteBuffers(1, &ibo);
    //glDeleteProgram(shader_program);
    
    glfwTerminate();

    return 0;
}

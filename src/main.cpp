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

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCharCallback(window, char_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    const char* vertex_shader_src =
        "#version 460 core\n"
        "layout (location = 0) in vec3 vpos;\n"
        "uniform mat4 projection;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = projection * vec4(vpos.xyz, 1.0f);\n"
        "}\0";
    
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

    const char* fragment_shader_src =
        "#version 460 core\n"
        "out vec4 ocolor;\n"
        "void main()\n"
        "{\n"
        "    ocolor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\0";
    
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

    mat4 projection = ortho(0.0f, k_window_width, 0.0f, k_window_height, -1.0f, 1.0f);
    glUseProgram(shader_program);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, (f32*)projection.rows);

    const u16 font_size = 32;
    const u16 vertex_count = (font_face.glyph->end_pts_of_countours[font_face.glyph->number_of_countours - 1] + 1);
    f32* vertices = (f32*)alloca(vertex_count * 3 * sizeof(f32));
    for (u16 i = 0, coord_idx = 0; i < vertex_count * 3; i += 3, ++coord_idx)
    {
#if 0
        const f32 scale = (f32)font_size / (f32)units_per_em;
        const f32 x_scaled = (f32)font_face.glyph->x_coordinates[i] * scale;
        const f32 y_scaled = (f32)font_face.glyph->y_coordinates[i] * scale;

        vertices[i + 0] = 2.0f * x_scaled / k_window_width - 1.0f;
        vertices[i + 1] = 1.0f - 2.0f * y_scaled / k_window_height;
        vertices[i + 2] = 0.0f;
#else
        vertices[i + 0] = (f32)font_face.glyph->x_coordinates[coord_idx] / 10 + k_window_width / 2;
        vertices[i + 1] = (f32)font_face.glyph->y_coordinates[coord_idx] / 10 + k_window_height / 2;
        vertices[i + 2] = 0.0f;
#endif
	}
    
    u32 vbo, ibo, vao;
    glGenBuffers(1, &vbo);
    //glGenBuffers(1, &ibo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(f32), vertices, GL_STATIC_DRAW);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);    

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);  

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
         
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(shader_program);
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, vertex_count);
        //glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteProgram(shader_program);
    
    glfwTerminate();

    return 0;
}

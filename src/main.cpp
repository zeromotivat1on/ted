#include "pch.h"
#include "font.h"
#include "file_reader.h"
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <malloc.h>

void char_callback(GLFWwindow* win, u32 character)
{
    msg_log("Window char (%c)", character);
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

    Arena font_arena = app_arena.subarena(MB(1));
    Font_Directory fd;
    file_handle font = open_file(font_path, FILE_OPEN_EXISTING, FILE_ACCESS_READ);
    
    File_Reader fr;
    fr.init(&font_arena, font);
        
    fd.read(&fr, &font_arena);
    close_file(font);

    msg_log("Font units_per_em (%u)", fd.head->units_per_em);
    msg_log("Font num_glyphs (%u)", fd.maxp->num_glyphs);
    msg_log("Font num_of_long_hor_metrics (%d)", fd.hhea->num_of_long_hor_metrics);
    
    //fd.cmap->print();
    //fd.format4->print();
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "ted", NULL, NULL);
    if (!window)
    {
        msg_critical("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCharCallback(window, char_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        msg_critical("Failed to initialize GLAD");
        return -1;
    }

    const char* vertex_shader_src =
        "#version 460 core\n"
        "layout (location = 0) in vec3 vpos;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(vpos.x, vpos.y, vpos.z, 1.0f);\n"
        "}\0";
    
    u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_src, NULL);
    glCompileShader(vertex_shader);

    s32 success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        msg_critical("Failed to compile vertex shader");
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
        msg_critical("Failed to compile fragment shader");
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
        msg_critical("Failed to link shader program");
        return -1;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    Simple_Glyph sg = fd.simple_glyph_from_char(&fr, &font_arena, 'g');
    sg.print();

#if 1
    const u16 font_size = 16;
    const u16 vertex_count = 3 * (sg.end_pts_of_countours[sg.glyph.number_of_countours - 1] + 1);
    f32* vertices = (f32*)malloc(vertex_count * sizeof(f32));
    for(u16 i = 0; i < vertex_count; i += 3)
    {
        const f32 scale = (f32)font_size / (f32)fd.head->units_per_em;
        const f32 x_scaled = (f32)sg.x_coordinates[i] * scale;
        const f32 y_scaled = (f32)sg.y_coordinates[i] * scale;
        
        vertices[i + 0] = 2.0f * x_scaled / 800.0f - 1.0f;
        vertices[i + 1] = 1.0f - 2.0f * y_scaled / 600.0f;
        vertices[i + 2] = 0.0f;
	}
#else
    const f32 vertices[] = {
        0.5f,  0.5f, 0.0f,  // top right
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f  // top left 
    };
#endif
    
    const u32 indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };  
            
    u32 vbo, ibo, vao;
    glGenBuffers(1, &vbo);
    //glGenBuffers(1, &ibo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count, vertices, GL_STATIC_DRAW);

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
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
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

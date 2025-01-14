#include "pch.h"
#include "gl.h"
#include "font.h"
#include "arena.h"
#include "matrix.h"
#include "memory.h"
#include <stdio.h>
#include <glad/glad.h>
#include <glfw/glfw3.h>

Arena heap_arena;
s16 font_size = 20;
Font* consola = null;
Font_Atlas* consola_ascii_atlas = null;
Font_Render_Context* consola_ascii_render_ctx = null;

// @Todo: remake display_str to be actual text editor compatible structure.
s32 pointer_pos = 0;
u32 display_str[512];

void char_callback(GLFWwindow* win, u32 character)
{
    //printf("Window char (%c) as u32 (%u)\n", character, character);
    
    if (character == '+')
    {
        font_size += 2;
        if (font_size > 256) font_size = 256;
        else rescale_font_atlas(&heap_arena, consola, consola_ascii_atlas, font_size);
    }
    else if (character == '-')
    {
        font_size -= 2;
        if (font_size <= 0) font_size = 2;
        else rescale_font_atlas(&heap_arena, consola, consola_ascii_atlas, font_size);
    }

    display_str[pointer_pos++] = character;
    if (pointer_pos >= sizeof(display_str)) pointer_pos = 0;
    //display_str[pointer_pos] = '\0';   
}

void key_callback(GLFWwindow* window, s32 key, s32 scancode, s32 action, s32 mods)
{
    //printf("Window key (%d) as char (%c)\n", key, (u32)key);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
  
    if (key == GLFW_KEY_ENTER && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        display_str[pointer_pos++] = '\n';
        if (pointer_pos >= sizeof(display_str)) pointer_pos = 0;
    }

    if (key == GLFW_KEY_BACKSPACE && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        display_str[--pointer_pos] = ' ';
        if (pointer_pos < 0) pointer_pos = 0;
    }
}

void framebuffer_size_callback(GLFWwindow* window, s32 width, s32 height)
{
    on_window_resize(consola_ascii_render_ctx, width, height);
    glViewport(0, 0, width, height);
}

int main()
{
    void* vm_base_addr = vm_base_addr_val;
    void* vm_core = vm_reserve(vm_base_addr, GB(1));

    constexpr u32 heap_size = MB(16);
    void* heap = vm_commit(vm_core, heap_size);
    heap_arena = create_arena(heap, heap_size);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "ted", null, null);
    if (!window)
    {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCharCallback(window, char_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    s32 win_w, win_h;
    glfwGetWindowSize(window, &win_w, &win_h);

    consola = push_struct(&heap_arena, Font);
    init_font(&heap_arena, consola, "C:/Windows/Fonts/Consola.ttf");

    consola_ascii_render_ctx = push_struct(&heap_arena, Font_Render_Context);
    init_font_render_context(&heap_arena, consola_ascii_render_ctx, win_w, win_h);
    
    consola_ascii_atlas = push_struct(&heap_arena, Font_Atlas);
    bake_font_atlas(&heap_arena, consola, consola_ascii_atlas, 32, 2079, font_size);
    
    f32 dt = 0.0f;
    f32 prev_time = (f32)glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {              
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwGetWindowSize(window, &win_w, &win_h);

        render_text(consola_ascii_render_ctx, consola_ascii_atlas, display_str, 1.0f, 0.0f, (f32)win_h - font_size, 1.0f, 1.0f, 1.0f);
        
        glfwSwapBuffers(window);
        glfwPollEvents();

        const f32 time = (f32)glfwGetTime();
        dt = time - prev_time;
        prev_time = time;
    }
    
    glfwTerminate();
    vm_release(vm_core);
    
    return 0;
}

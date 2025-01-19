#include "pch.h"
#include "ted.h"
#include "arena.h"
#include "memory.h"
#include <stdio.h>
#include <glad/glad.h>
#include <glfw/glfw3.h>

#if 0
Arena heap_arena;
s16 font_size = 20;
Font* consola = null;
Font_Atlas* consola_ascii_atlas = null;
Font_Render_Context* consola_ascii_render_ctx = null;
Gap_Buffer* display_buffer = null;
#endif

int main()
{    
    void* vm_base_addr = vm_base_addr_val;
    void* vm_core = vm_reserve(vm_base_addr, GB(1));

    constexpr u32 heap_size = MB(16);
    void* heap = vm_commit(vm_core, heap_size);
    Arena heap_arena = create_arena(heap, heap_size);

#if 1
    auto* ted = push_struct(&heap_arena, Ted_Context);
    init_ted_context(&heap_arena, ted);
    create_window(ted, 800, 600, "ted");
    load_font(ted, "C:/Windows/Fonts/Consola.ttf");
    init_render_context(ted);
    bake_font(ted, 0, 127, 6, 64, 4);

    const s16 main_buffer = create_buffer(ted);
    set_active_buffer(ted, main_buffer);
#else
    
    display_buffer = push_struct(&heap_arena, Gap_Buffer);
    init(display_buffer, 8);

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
    glfwSetDropCallback(window, drop_callback);

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
    bake_font_atlas(&heap_arena, consola, consola_ascii_atlas, 0, 127, font_size);
    
    u32 display_str[512] = {0};
    u32 display_debug_str[512] = {0};
    u32 display_debug_data_str[256] = {0};
    char debug_data_str[256] = {0};
    s64 render_text_size = 0;
    
    const vec3 bg_color = {2.0f / 255.0f, 26.0f / 255.0f, 25.0f / 255.0f};
    const vec3 text_color = {255.0f / 255.0f, 220.0f / 255.0f, 194.0f / 255.0f};
#endif

    f32 dt = 0.0f;
    f32 prev_time = (f32)glfwGetTime();
    
    while (alive(ted))
    {
#if 1
        update_frame(ted);
#else
        glClearColor(bg_color.r, bg_color.g, bg_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwGetWindowSize(window, &win_w, &win_h);

        render_text_size = fill_utf32(display_buffer, display_str);
        render_text(consola_ascii_render_ctx, consola_ascii_atlas, display_str, render_text_size, 1.0f, 0.0f, (f32)win_h - font_size, text_color.r, text_color.g, text_color.b);

#if 0
        render_text_size = fill_debug_utf32(display_buffer, display_debug_str);
        render_text(consola_ascii_render_ctx, consola_ascii_atlas, display_debug_str, render_text_size, 1.0f, 0.0f, (f32)win_h * 0.5f - font_size, text_color.r, text_color.g, text_color.b);
#endif
        
        render_text_size = sprintf(debug_data_str, "cursor_pos=%lld, end=%lld, gap_start=%lld, gap_end=%lld\n", display_buffer->cursor - display_buffer->start, display_buffer->end - display_buffer->start, display_buffer->gap_start - display_buffer->start, display_buffer->gap_end - display_buffer->start);
            
        for (u32 i = 0; i < render_text_size; ++i)
            display_debug_data_str[i] = debug_data_str[i];
        
        render_text(consola_ascii_render_ctx, consola_ascii_atlas, display_debug_data_str, render_text_size, 1.0f, 0.0f, font_size, text_color.r, text_color.g, text_color.b);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
#endif
        
        const f32 time = (f32)glfwGetTime();
        dt = time - prev_time;
        prev_time = time;
    }
    
    destroy(ted);
    vm_release(vm_core);
    
    return 0;
}

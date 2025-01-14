#include "pch.h"
#include "gl.h"
#include "font.h"
#include "arena.h"
#include "matrix.h"
#include "memory.h"
#include <stdio.h>
#include <glad/glad.h>
#include <glfw/glfw3.h>

const char* lorem = "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.\nLorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.\nLorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.\nLorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.\nLorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.\nLorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.\nLorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.\nLorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.\nLorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.\nLorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.\nLorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.\nAt vero eos et accusam et justo duo dolores et ea rebum.\nStet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.";

mat4 projection = mat4_ortho(0.0f, 1920.0f, 0.0f, 1080.0f, -1.0f, 1.0f);
Font consola;
s16 font_size = 20;
GL_Text_Render_Context* ascii_render_ctx = null;

s32 pointer_pos = 0;
char display_str[512];

void char_callback(GLFWwindow* win, u32 character)
{
    //printf("Window char (%c)\n", character);
    
    if (character == '+')
    {
        font_size += 2;
        if (font_size > 256) font_size = 256;
        else gl_update_glyph_bitmaps(&consola, ascii_render_ctx->glyph_cache, font_size);
    }
    else if (character == '-')
    {
        font_size -= 2;
        if (font_size <= 0) font_size = 2;
        else gl_update_glyph_bitmaps(&consola, ascii_render_ctx->glyph_cache, font_size);
    }
    
    display_str[pointer_pos++] = (char)character;
    display_str[pointer_pos] = '\0';   
}

void key_callback(GLFWwindow* window, s32 key, s32 scancode, s32 action, s32 mods)
{
    //printf("Window key (%d) as char (%c)\n", key, (u32)key);
    
    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
    {
          display_str[pointer_pos++] = '\n';
          display_str[pointer_pos] = '\0';
    }
}

void framebuffer_size_callback(GLFWwindow* window, s32 width, s32 height)
{
    projection = mat4_ortho(0.0f, (f32)width, 0.0f, (f32)height, -1.0f, 1.0f);
    ascii_render_ctx->u_projection.val = projection;
    glViewport(0, 0, width, height);
}

int main()
{
    void* vm_base_addr = vm_base_addr_val;
    void* vm_core = vm_reserve(vm_base_addr, GB(1));

    constexpr u32 heap_size = MB(64);
    void* heap = vm_commit(vm_core, heap_size);
    Arena heap_arena = create_arena(heap, heap_size);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "ted", null, null);
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

    init_font(&heap_arena, &consola, "C:/Windows/Fonts/Consola.ttf");
    ascii_render_ctx = gl_create_text_render_context(&heap_arena, &consola, 32, 127, font_size, 1920, 1080);

    Font_Render_Context consola_ascii_render_ctx;
    init_font_render_context(&heap_arena, &consola_ascii_render_ctx);

    Font_Atlas consola_ascii_atlas;
    bake_font_atlas(&heap_arena, &consola, &consola_ascii_atlas, 32, 127, 20);
    
    Font_Render_Command consola_ascii_render_cmd;
    consola_ascii_render_cmd.text = lorem;
    consola_ascii_render_cmd.projection = &projection;
    consola_ascii_render_cmd.x = 100.0f;
    consola_ascii_render_cmd.y = 200.0f;
    consola_ascii_render_cmd.r = 1.0f;
    consola_ascii_render_cmd.g = 1.0f;
    consola_ascii_render_cmd.b = 1.0f;

    Font_Render_Command consola_ascii_dynamic_render_cmd = consola_ascii_render_cmd;
    consola_ascii_dynamic_render_cmd.text = display_str;
    consola_ascii_dynamic_render_cmd.x = 0.0f;
    consola_ascii_dynamic_render_cmd.y = 500.0f;
    
    f32 dt = 0.0f;
    f32 prev_time = (f32)glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
                
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        s32 window_w, window_h;
        glfwGetWindowSize(window, &window_w, &window_h);

        // Render technique comparison.
#if 0
        // Draw each glyph separately | debug ~ 290fps | release ~ 325fps
        vec2 text_pos = {0.0f, (f32)window_h};
        //char str[64];
        //sprintf(str, "frame %.2fms %.0ffps", dt * 1000.0f, 1.0f / dt);
        text_pos.y -= (f32)font_size;
        gl_render_text(ascii_render_ctx, text_pos, lorem);
#else
        // Draw glyphs in batch (128) | debug ~ 1850fps | release ~ 4000fps
        consola_ascii_render_cmd.x = 0.0f;
        consola_ascii_render_cmd.y = (f32)window_h - font_size;
        render_text(&consola_ascii_render_ctx, &consola_ascii_atlas, &consola_ascii_render_cmd);
#endif
        
        //text_pos.y -= (f32)font_size;
        //gl_render_text(ascii_render_ctx, text_pos, display_str);
        //render_text(&consola_ascii_render_ctx, &consola_ascii_atlas, &consola_ascii_dynamic_render_cmd);

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

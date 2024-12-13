#include "pch.h"
#include "font.h"
#include "file_reader.h"

void load_font(Arena* arena, const char* path)
{
    file_handle font = open_file(path, FILE_OPEN_EXISTING, FILE_ACCESS_READ);
    
    File_Reader fr;
    fr.init(arena, font);

    Font_Directory fd;
    fd.read(arena, &fr);
    fd.print();

    const u16 glyph_char = 'A';
    const u16 glyph_idx = fd.glyph_index(glyph_char);
    msg_log("Glyph Index of (%c) is (%u)", glyph_char, glyph_idx);
    msg_log("Simple Glyph (%c)", glyph_char);
    Glyph_Outline go = fd.glyph_outline(arena, &fr, glyph_idx);
    go.print();

    close_file(font);
}

void on_window_char(Window* win, u32 character)
{
    msg_log("Window char (%c)", character);
}

int main()
{
    #if 1
    const char* font_path = "C:/Users/admin/Downloads/Envy_Code_R_PR7/Envy_Code_R.ttf";
    #else
    const char* font_path = "C:/Users/admin/Downloads/consola.ttf";
    #endif
    
    constexpr u64 k_arena_size = MB(32);
    Arena global_arena = create_arena(malloc(k_arena_size), k_arena_size);

    Arena font_arena = create_arena(global_arena.push(MB(4)), MB(4));
    load_font(&font_arena, font_path);
    
#if 0
    Window_Info winfo;
    winfo.title = "ted";
    winfo.width = 1080;
    winfo.height = 720;
    winfo.x = 100;
    winfo.y = 100;
    
    Window win;
    const bool win_res = init_window(&win, &winfo);
    ASSERT(win_res);

    set_window_char_callback(&win, on_window_char);
    show_window(&win);

    while(is_window_active(&win))
    {
        update_window(&win);
    }
#endif
    
    return 0;
}

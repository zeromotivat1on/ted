#include "pch.h"
#include "ted.h"
#include "memory.h"
#include "gap_buffer.h"

int main()
{
    void* vm_base_addr = vm_base_addr_val;
    void* vm_core = vm_reserve(vm_base_addr, GB(1));

    constexpr u32 heap_size = MB(16);
    void* heap = vm_commit(vm_core, heap_size);

    Ted_Context ted;
    init_ted_context(&ted, heap, heap_size);
    create_window(&ted, 800, 600, 4, 32);
    load_font(&ted, "C:/Windows/Fonts/Consola.ttf");
    init_render_context(&ted);
    bake_font(&ted, 0, 127, 6, 128, 4);
    ted.active_atlas_idx = 6;

    const s16 buffer_idx = create_buffer(&ted);
    set_active_buffer(&ted, buffer_idx);
    push_str(&ted, buffer_idx,
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
             "abcdefghijklmnopqrstuvwxyz\n"
             "`1234567890-=[]\\;',./\n"
             "~!@#$%^&*()_+{}|:\"<>?\n",
             27 + 27 + 22 + 22);
    
    while (alive(&ted))
    {
        update_frame(&ted);
    }
    
    destroy(&ted);
    vm_release(vm_core);
    
    return 0;
}

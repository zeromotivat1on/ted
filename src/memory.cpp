#include "pch.h"
#include "memory.h"
#include <windows.h>

#ifndef WIN32
#error "Only win32 is supported for now"
#endif

void* vm_reserve(void* addr, u64 size)
{
    assert(size > 0);
    return VirtualAlloc(addr, size, MEM_RESERVE, PAGE_READWRITE);
}

void* vm_commit(void* vm, u64 size)
{
    assert(vm);
    assert(size > 0);
    return VirtualAlloc(vm, size, MEM_COMMIT, PAGE_READWRITE);
}

bool vm_decommit(void* vm, u64 size)
{
    assert(vm);
    assert(size > 0);
    return VirtualFree(vm, size, MEM_DECOMMIT);
}

bool vm_release(void* vm)
{
    assert(vm);
    return VirtualFree(vm, 0, MEM_RELEASE);
}

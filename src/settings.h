#pragma once

inline constexpr s32 MAX_TAB_SIZE = 16;

struct Ted_Settings
{
    s32 tab_size;
};

inline Ted_Settings ted_settings;

const char* tab_as_space_string(); // null-terminated

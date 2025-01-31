#include "pch.h"
#include "settings.h"
#include <string.h>

const char* tab_as_space_string()
{
    static char spaces[MAX_TAB_SIZE + 1];
    assert(ted_settings.tab_size <= MAX_TAB_SIZE);
    memset(spaces, ' ', ted_settings.tab_size);
    spaces[ted_settings.tab_size] = '\0';
    return spaces;
}

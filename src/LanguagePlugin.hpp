#pragma once

#include <cstdint>

struct LanguagePlugin
{
    int32_t sub_language;
    int32_t language;
    char name[260];
    char path[260];
    char information[260];
};

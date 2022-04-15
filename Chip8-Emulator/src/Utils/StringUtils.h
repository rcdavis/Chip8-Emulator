#pragma once

#include <string>

namespace StringUtils
{
    template <typename... Args>
    std::string Format(const char* const format, Args... args)
    {
        char buffer[128] = {};
        snprintf(buffer, sizeof(buffer), format, args...);
        return buffer;
    }
}

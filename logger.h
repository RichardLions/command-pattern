#pragma once

#include <iostream>
#include <string_view>

namespace Logger
{
    static void LogMessage(const std::string_view message)
    {
        std::cout << message << "\n";
    }
}

#pragma once

#include <iostream>
#include <string_view>

namespace Logger
{
    /// Print the message followed by a return
    static void LogMessage(const std::string_view message)
    {
        std::cout << message << "\n";
    }
}

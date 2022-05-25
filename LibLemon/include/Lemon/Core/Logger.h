#pragma once

#include <iostream>
#include <mutex>

#define FMT_EXCEPTIONS 0
#define FMT_HEADER_ONLY 1

#include <fmt/core.h>
#include <fmt/format.h>

namespace Lemon {
namespace Logger {

const char* GetProgramName();

template <typename... Args> inline void Debug(fmt::format_string<Args...> f, Args&&... args) {
    fmt::print(stderr, "[{}] [Debug] ", GetProgramName());
    fmt::vprint(stderr, f, fmt::make_format_args(args...));
    fmt::print(stderr, "\n");
}

template <typename... Args> inline void Warning(fmt::format_string<Args...> f, Args&&... args) {
    fmt::print(stderr, "[{}] [Warning] ", GetProgramName());
    fmt::vprint(stderr, f, fmt::make_format_args(args...));
    fmt::print(stderr, "\n");
}

template <typename... Args> inline void Error(fmt::format_string<Args...> f, Args&&... args) {
    fmt::print(stderr, "[{}] [Error] ", GetProgramName());
    fmt::vprint(stderr, f, fmt::make_format_args(args...));
    fmt::print(stderr, "\n");
}

} // namespace Logger
} // namespace Lemon

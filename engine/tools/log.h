#pragma once

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <fmt/xchar.h>

#include <cstdint>
#include <source_location>
#include <string>
#include <unordered_map>

enum class LogSeverity : uint8_t
{
    Display = 0,
    Log,
    Warning,
    Error,
    Fatal
};

// The class to use to implement categories
class LogCategory
{
public:
    explicit LogCategory(
        const std::string&& category_name, LogSeverity severity = LogSeverity::Log
    );

    [[nodiscard]] auto can_log(LogSeverity severity) const -> bool;

    const std::string category_name;
    const LogSeverity severity;
    // right now it doesn't really have much info in it...
};

// Defines a new category for the logger.
// Takes an optional LogSeverity parameter to define the minimum supported severity.
#define MakeCategory(Name, ...) \
    inline static LogCategory Name##Category = LogCategory(#Name, __VA_ARGS__)

template <typename... Args>
class Log
{
public:
    Log(const LogCategory& category,
        const LogSeverity severity,
        const std::string& message,
        Args&&... args,
        const std::source_location& location = std::source_location::current())
    {
        log(category, severity, location, message, args...);
    }

private:
    static void log(
        const LogCategory& category,
        const LogSeverity severity,
        const std::source_location& location,
        const std::string& message,
        Args... args
    )
    {
        if (!category.can_log(severity)) return;

        // always present
        const auto cat = get_category_format(category, severity);
        // might be empty, depending on the category
        const auto loc = get_location_format(location, severity);
        // the actual message...
        const auto msg = get_message_format(message, severity, args...);
        // print the final result (we must do this section formatting sequence because somehow it
        // didn't format the whole concatenated sequence)
        fmt::print(fmt::runtime(cat + loc + msg + "\n"), args...);
    }

    inline static auto get_message_format(
        const std::string& message, const LogSeverity severity, Args... args
    ) -> std::string
    {
        return fmt::format(
            fmt::fg(colors[severity]), fmt::runtime( ": " + message), args...
        );
    }

    inline static auto get_category_format(const LogCategory& category, const LogSeverity severity)
        -> std::string
    {
        return fmt::format(
            fmt::emphasis::bold | fmt::fg(colors[severity]),
            fmt::runtime("[" + category.category_name + "]")
        );
    };

    inline static auto get_location_format(
        const std::source_location& location, const LogSeverity severity
    ) -> std::string
    {
        // only print additional info when there is an error
        if (severity <= LogSeverity::Warning) return "";

        return fmt::format(
            fmt::emphasis::bold | fmt::fg(colors[severity]),
            " [{}({},{})]",
            location.file_name(),
            location.line(),
            location.column()
        );
    }

    inline static std::unordered_map<LogSeverity, fmt::color> colors{
        {LogSeverity::Display, fmt::color::light_steel_blue},
        {LogSeverity::Log, fmt::color::light_gray},
        {LogSeverity::Warning, fmt::color::gold},
        {LogSeverity::Error, fmt::color::orange_red},
        {LogSeverity::Fatal, fmt::color::crimson},
    };
};
template <typename... Args>
Log(const LogCategory& category, LogSeverity severity, const std::string& message, Args&&...)
    -> Log<Args...>;

// define a few categories for everyone to use
MakeCategory(LogTemp);

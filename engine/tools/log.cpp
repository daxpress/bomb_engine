#include "log.h"

LogCategory::LogCategory(const std::string&& category_name, const LogSeverity severity)
    : category_name(category_name), severity(severity)
{
}

auto LogCategory::can_log(const LogSeverity severity) const -> bool
{
    return this->severity <= severity;
}

void DefaultTerminalDevice::print_message(
    LogSeverity severity,
    const std::string& category,
    const std::string& location,
    const std::string& message
)
{
    // style the sections
    const auto color = fmt::fg(colors.at(severity));
    const auto cat = fmt::format(fmt::emphasis::bold | color, fmt::runtime(category));
    const auto loc = fmt::format(fmt::emphasis::bold | color, fmt::runtime(location));
    const auto msg = fmt::format(color, fmt::runtime(message));
    // print everything!
    const auto out = severity >= LogSeverity::Error ? stderr : stdout;
    fmt::print(out, fmt::runtime(cat + loc + msg));
}

void DefaultFileDevice::print_message(
    LogSeverity severity,
    const std::string& category,
    const std::string& location,
    const std::string& message
)
{
    if (severity <= LogSeverity::Log) return;
    // to be implemented...
}
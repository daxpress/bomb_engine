#include "log.h"
#include <fmt/std.h>

LogCategory::LogCategory(
    const std::string&& category_name, const LogSeverity severity
)
    : category_name(category_name), severity(severity)
{
}

auto LogCategory::can_log(const LogSeverity severity) const -> bool
{
    return this->severity <= severity;
}
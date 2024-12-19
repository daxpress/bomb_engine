#include "log.h"

#include <fstream>
#include <mutex>
#include <syncstream>
#include <thread>

LogCategory::LogCategory(const std::string&& category_name, const LogSeverity severity)
    : m_category_name(category_name), m_severity(severity)
{
}

auto LogCategory::can_log(const LogSeverity severity) const -> bool
{
    return m_severity <= severity;
}

void DefaultTerminalDevice::print_message(
    const LogSeverity severity,
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

static auto get_log_type_string(const LogSeverity severity) -> std::string
{
    switch (severity)
    {
        case LogSeverity::Display:
            return "Display";
        case LogSeverity::Log:
            return "Log";
        case LogSeverity::Warning:
            return "Warning";
        case LogSeverity::Error:
            return "Error";
        case LogSeverity::Fatal:
            return "Fatal";
    }
    return "";
}

void DefaultFileDevice::print_message(
    const LogSeverity severity,
    const std::string& category,
    const std::string& location,
    const std::string& message
)
{
    if (severity < LogSeverity::Log) return;
    // make dir if not present
    std::filesystem::create_directory(log_dir);
    // open fstream
    auto log_file = std::ofstream(log_path, std::ios::app);
    if (!log_file.is_open()) return;

    // // wrap with osyncstream to ensure it is thread safe
    auto sync_stream = std::osyncstream(log_file);
    const auto log_type = get_log_type_string(severity);
    // get the current timestamp for additional info in the log
    const auto time =
        std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now())
            .get_local_time();
    fmt::print(
        sync_stream, fmt::runtime("[{}] - {} [{}]{}{}"), time, category, log_type, location, message
    );
}
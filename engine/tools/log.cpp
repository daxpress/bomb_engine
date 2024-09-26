#include "log.h"

#include <fstream>
#include <mutex>
#include <syncstream>
#include <thread>

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
    if (severity < LogSeverity::Log) return;
    // make dir if not present
    std::filesystem::create_directory(log_dir);
    // open fstream
    auto log_file = std::ofstream(log_path, std::ios::app);
    if (!log_file.is_open()) return;

    // // wrap with osyncstream to ensure it is thread safe
    auto sync_stream = std::osyncstream(log_file);
    // get the current timestamp for additional info in the log
    const auto time =
        std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now())
            .get_local_time();
    fmt::print(sync_stream, fmt::runtime("[{}]: {}{}{}"), time, category, location, message);
}
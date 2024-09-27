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

#pragma region Log Helper Classes

// By default:
// * Display only prints the output to terminal
// * Log prints to terminal and to a log file
// * Warning prints to terminal and to a log file
// * Error prints to both terminal and file, adding file, line and column info
// * Fatal behaves like Error but crashes the session as well
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
        const std::string&& category_name, LogSeverity severity = LogSeverity::Display
    );

    [[nodiscard]] auto can_log(LogSeverity severity) const -> bool;

    const std::string m_category_name;
    const LogSeverity m_severity;
    // right now it doesn't really have much info in it...
};

// Defines a new category for the logger.
// Takes an optional LogSeverity parameter to define the minimum supported severity.
#define MakeCategory(Name, ...) \
    inline static LogCategory Name##Category = LogCategory(#Name, __VA_ARGS__)

#pragma endregion

#pragma region Log Devices
namespace bomb_engine
{
// Interface to use for outputting to custom devices (remote terminals, viewports etc...)
class ILogDevice
{
public:
    virtual ~ILogDevice() = default;
    virtual void print_message(
        LogSeverity severity,
        const std::string& category,
        const std::string& location,
        const std::string& message
    ) = 0;
};

}  // namespace bomb_engine

class DefaultTerminalDevice final : public bomb_engine::ILogDevice
{
public:
    void print_message(
        LogSeverity severity,
        const std::string& category,
        const std::string& location,
        const std::string& message
    ) override;

private:
    inline const static std::unordered_map<LogSeverity, fmt::color> colors{
        {LogSeverity::Display, fmt::color::light_steel_blue},
        {LogSeverity::Log, fmt::color::light_gray},
        {LogSeverity::Warning, fmt::color::gold},
        {LogSeverity::Error, fmt::color::orange_red},
        {LogSeverity::Fatal, fmt::color::crimson},
    };
};

class DefaultFileDevice final : public bomb_engine::ILogDevice
{
public:
    void print_message(
        LogSeverity severity,
        const std::string& category,
        const std::string& location,
        const std::string& message
    ) override;

private:
    // static: it will compose the log path at the beginning of the session, it will remain the same
    // throughout it.

    // make log directory
    inline static auto timepoint =
        std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now())
            .get_local_time();
    inline static auto log_dir =
        std::filesystem::path(std::filesystem::current_path().string() + "/logs").make_preferred();
    // make log path
    inline static auto log_path =
        std::filesystem::path(
            fmt::format(fmt::runtime("{}/log_{:%F_%H.%M}.txt"), log_dir, timepoint)
        )
            // convert the separators to the system ones
            .make_preferred();
};

#pragma endregion

// refer to https://fmt.dev/latest/syntax/#chrono-format-specifications for the formatting
// specifications
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

    // use this function to add a new device in the output devices list
    static void add_device(std::shared_ptr<bomb_engine::ILogDevice> device)
    {
        devices.emplace_back(device);
    }

    // use this function to remove a new device in the output devices list
    static void remove_device(std::shared_ptr<bomb_engine::ILogDevice> device)
    {
        std::erase(devices, device);
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
        // check if category accepts this severity
        if (!category.can_log(severity)) return;

        // gather the formatted pieces for the message
        const auto cat = get_category_format(category);
        const auto loc = get_location_format(location, severity);
        const auto msg = get_message_format(message, args...);

        // print using the available devices
        std::for_each(
            devices.begin(),
            devices.end(),
            [&](const std::shared_ptr<bomb_engine::ILogDevice>& device)
            // pass the pieces to the devices in the list, they will handle them as needed
            { device->print_message(severity, cat, loc, msg + "\n"); }
        );

        // lastly crash if severity is FATAL
        if (severity == LogSeverity::Fatal) std::terminate();
    }

    inline static auto get_message_format(const std::string& message, Args... args) -> std::string
    {
        return fmt::format(fmt::runtime(": " + message), args...);
    }

    inline static auto get_category_format(const LogCategory& category) -> std::string
    {
        return fmt::format(fmt::runtime("[" + category.m_category_name + "]"));
    };

    inline static auto get_location_format(
        const std::source_location& location, const LogSeverity severity
    ) -> std::string
    {
        // only print additional info when there is an error
        if (severity <= LogSeverity::Warning) return "";

        return fmt::format(
            " [{}({},{})]", location.file_name(), location.line(), location.column()
        );
    }

private:
    // initialize with default devices
    inline static std::vector<std::shared_ptr<bomb_engine::ILogDevice>> devices{
        std::make_shared<DefaultTerminalDevice>(), std::make_shared<DefaultFileDevice>()
    };
};
template <typename... Args>
Log(const LogCategory& category, LogSeverity severity, const std::string& message, Args&&...)
    -> Log<Args...>;

// define a few categories for everyone to use
MakeCategory(LogTemp);

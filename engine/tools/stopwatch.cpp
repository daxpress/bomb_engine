#include "stopwatch.h"

namespace BE_NAMESPACE
{
Stopwatch::Stopwatch() { m_start = std::chrono::system_clock::now(); }
auto Stopwatch::elapsed() const -> std::chrono::duration<double>
{
    const auto end = std::chrono::system_clock::now();
    return (end - m_start);
}
auto Stopwatch::lap() -> std::chrono::duration<double>
{
    const auto this_lap = elapsed();
    restart();
    return this_lap;
}
auto Stopwatch::restart() -> void { m_start = std::chrono::system_clock::now(); }
}  // namespace BE_NAMESPACE
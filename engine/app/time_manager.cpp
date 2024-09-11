#include "time_manager.h"

namespace BE_NAMESPACE
{
void TimeManager::start()
{
    m_start_time = std::chrono::high_resolution_clock::now();
    m_current_time = m_start_time;
}
auto TimeManager::tick() -> float
{
    // get last frame time
    auto last_tick = m_current_time;
    // update frame time to current
    m_current_time = std::chrono::high_resolution_clock::now();
    // get the difference in secs
    m_delta_time =
        std::chrono::duration<float, std::chrono::seconds::period>(m_current_time - last_tick);

    return delta_time();
}
auto TimeManager::delta_time() -> float { return m_delta_time.count(); }
auto TimeManager::since_start() -> float
{
    return std::chrono::duration<float, std::chrono::seconds::period>(m_current_time - m_start_time)
        .count();
}
}  // namespace BE_NAMESPACE
#pragma once

#include <chrono>

#include "dispatcher.h"

namespace BE_NAMESPACE
{
class TimeManager
{
private:
    // very simple time manager at the moment, with easy events using funcs
    TimeManager() = default;
    ~TimeManager() = default;

    friend class App;

    void start();
    auto tick() -> float;

public:
    auto delta_time() -> float;
    auto since_start() -> float;

private:
    // should also include timers and delays

    std::chrono::steady_clock::time_point m_start_time;
    std::chrono::steady_clock::time_point m_current_time;

    // caching delta_time in duration to enable duration_casts
    std::chrono::duration<float, std::chrono::seconds::period> m_delta_time;
};
}  // namespace BE_NAMESPACE
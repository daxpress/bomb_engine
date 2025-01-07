#pragma once
#include <chrono>
#include <stack>

#include "../macros.h"

namespace BE_NAMESPACE
{
class Stopwatch
{
public:
    Stopwatch();
    [[nodiscard]] auto elapsed() const -> std::chrono::duration<double>;
    [[nodiscard]] auto lap() -> std::chrono::duration<double>;
    auto restart() -> void;

private:
    std::chrono::time_point<std::chrono::system_clock> m_start;
};

}  // namespace BE_NAMESPACE
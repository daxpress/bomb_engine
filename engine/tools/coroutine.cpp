#include "coroutine.h"
#include "log.h"

MakeCategory(Coroutine);

namespace BE_NAMESPACE
{
void Coroutine::promise_type::unhandled_exception() noexcept
{
    Log(CoroutineCategory, LogSeverity::Fatal, "A coroutine threw an exception!");
}
}  // namespace BE_NAMESPACE
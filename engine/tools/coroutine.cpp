#include "coroutine.h"

#include "log.h"

MakeCategory(Coroutine);

namespace BE_NAMESPACE
{
auto Coroutine::promise_type::get_return_object() -> Coroutine
{
    return Coroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
}
void Coroutine::promise_type::unhandled_exception() noexcept
{
    Log(CoroutineCategory, LogSeverity::Fatal, "A coroutine threw an exception!");
}
Coroutine::~Coroutine()
{
    if (handle)
    {
        handle.destroy();
    }
}
auto Coroutine::operator=(Coroutine&& other) noexcept -> Coroutine&
{
    if (this != &other)
    {
        handle = std::exchange(other.handle, nullptr);
    }
    return *this;
}
auto coroutine_awaitable::await_ready() const noexcept -> bool
{
    // query status
    return future &&
           future.value().wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
}
void coroutine_awaitable::await_suspend(
    std::coroutine_handle<Coroutine::promise_type> h
) const noexcept
{
    auto& promise = h.promise();
    if (future)
    {
        promise.set_future(future.value());
    }
}
}  // namespace BE_NAMESPACE
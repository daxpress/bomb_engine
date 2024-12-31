#pragma once

#include <algorithm>
#include <coroutine>

#include "../macros.h"

namespace BE_NAMESPACE
{

struct Coroutine
{
    // define the promise_type for the coroutine
    struct promise_type
    {
        auto get_return_object() -> Coroutine
        {
            return Coroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        // still debating if using never or always
        auto initial_suspend() noexcept -> std::suspend_always { return {}; }
        [[nodiscard]] auto final_suspend() const noexcept -> std::suspend_always { return {}; }

        void return_void() noexcept {}
        void unhandled_exception() noexcept;
    };

    explicit Coroutine(std::coroutine_handle<promise_type> from_promise) : handle(from_promise) {};

    Coroutine() = default;
    ~Coroutine()
    {
        if (handle)
        {
            handle.destroy();
        }
    }
    Coroutine(Coroutine&& other) noexcept : handle(std::exchange(other.handle, nullptr)) {}
    auto operator=(Coroutine&& other) noexcept -> Coroutine&
    {
        if (this != &other)
        {
            handle = std::exchange(other.handle, nullptr);
        }
        return *this;
    }

    std::coroutine_handle<promise_type> handle;
};
}  // namespace BE_NAMESPACE
#pragma once

#include <algorithm>
#include <coroutine>
#include <future>
#include <memory>

#include "../macros.h"

namespace BE_NAMESPACE
{
using coroutine_future = std::shared_future<void>;

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

        void set_future(std::optional<coroutine_future>&& in_f) { future = std::move(in_f); }

        std::optional<coroutine_future> future = std::nullopt;
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

struct coroutine_awaitable
{
    std::optional<coroutine_future> future;

    explicit coroutine_awaitable(coroutine_future&& future)
        : future(std::make_optional<coroutine_future>(std::move(future)))
    {
    }

    bool await_ready() const noexcept
    {
        // query status
        return future &&
               future.value().wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
    }
    void await_suspend(std::coroutine_handle<Coroutine::promise_type> h) const noexcept
    {
        auto& promise = h.promise();
        if (future)
        {
            promise.set_future(std::move(future.value()));
        }
        std::thread(
            [this, h]()
            {
                future->wait();  // Wait for the future to complete
            }
        ).detach();
    }

    void await_resume() const noexcept {}
};

}  // namespace BE_NAMESPACE
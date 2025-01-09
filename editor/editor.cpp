// editor.cpp : Defines the entry point for the application.
// right now it acts more like a sandbox to put everything together and test the functionalities.

#include <future>

#include "app/app.h"
#include "pybomb_cpp.h"
#include "tools/coroutine.h"
#include "tools/task_graph.h"

constexpr bool debug = _DEBUG;

MakeCategory(Editor);

auto coroutine() -> bomb_engine::Coroutine
{
    Log(LogTempCategory, LogSeverity::Display, "Coroutine started");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    Log(LogTempCategory, LogSeverity::Display, "Coroutine paused");
    co_await std::suspend_always{};
    Log(LogTempCategory, LogSeverity::Display, "Coroutine resumed");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    Log(LogTempCategory, LogSeverity::Display, "Coroutine ended");
    co_return;
}
auto coroutine2() -> bomb_engine::Coroutine
{
    Log(LogTempCategory, LogSeverity::Display, "Coroutine 2 started");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    Log(LogTempCategory, LogSeverity::Display, "Coroutine 2 paused");
    co_await std::suspend_always{};
    Log(LogTempCategory, LogSeverity::Display, "Coroutine 2 resumed");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    Log(LogTempCategory, LogSeverity::Display, "Coroutine 2 ended");
    co_return;
}
auto coroutine3() -> bomb_engine::Coroutine
{
    Log(LogTempCategory, LogSeverity::Display, "Coroutine 3 started");
    std::this_thread::sleep_for(std::chrono::seconds(6));
    Log(LogTempCategory, LogSeverity::Display, "Coroutine 3 paused");
    co_await std::suspend_always{};
    Log(LogTempCategory, LogSeverity::Display, "Coroutine 3 resumed");
    std::this_thread::sleep_for(std::chrono::seconds(0));
    Log(LogTempCategory, LogSeverity::Display, "Coroutine 3 ended");
    co_return;
}

bomb_engine::coroutine_future simulate_task()
{
    Log(LogTempCategory, LogSeverity::Display, "Simulating task");
    return std::async(
               std::launch::async,
               []
               {
                   std::this_thread::sleep_for(std::chrono::seconds(5));
                   Log(LogTempCategory, LogSeverity::Display, "Simulated task ended");
               }
    ).share();
}

auto super_slow_async_work() -> bomb_engine::Coroutine
{
    Log(LogTempCategory, LogSeverity::Display, "Super slow work started");
    auto awaitable = bomb_engine::coroutine_awaitable(std::move(simulate_task()));
    co_await awaitable;
    Log(LogTempCategory, LogSeverity::Display, "Super slow work ended");
    auto second_awaitable = bomb_engine::coroutine_awaitable(std::move(simulate_task()));
    co_await second_awaitable;
    Log(LogTempCategory, LogSeverity::Display, "Second Super slow work ended");
    co_return;
}

auto main() -> int
{
    auto app = bomb_engine::App(debug);
    app.start();

    bomb_engine::TaskGraph tasks;
    auto task1 = tasks.add_task(
        []
        {
            Log(LogTempCategory, LogSeverity::Display, "Task 1 started");
            std::this_thread::sleep_for(std::chrono::seconds(5));
            Log(LogTempCategory, LogSeverity::Display, "Task 1 ended");
        }
    );
    auto task2 = tasks.add_task(
        []
        {
            Log(LogTempCategory, LogSeverity::Display, "Task 2 started");
            std::this_thread::sleep_for(std::chrono::seconds(3));
            Log(LogTempCategory, LogSeverity::Display, "Task 2 ended");
        }
    );
    auto task3 = tasks.add_task(
        []
        {
            Log(LogTempCategory, LogSeverity::Display, "Task 3 started");
            std::this_thread::sleep_for(std::chrono::seconds(2));
            Log(LogTempCategory, LogSeverity::Display, "Task 3 ended");
        }
    );
    const auto task4 = tasks.add_task(
        []
        {
            Log(LogTempCategory, LogSeverity::Display, "Task 4 started");
            std::this_thread::sleep_for(std::chrono::seconds(2));
            Log(LogTempCategory, LogSeverity::Display, "Task 4 ended");
        }
    );

    const auto task6 = tasks.add_task(coroutine());
    const auto task7 = tasks.add_task(coroutine2());
    const auto task8 = tasks.add_task(coroutine3());
    const auto task9 = tasks.add_task(super_slow_async_work());
    task1.before(task2);

    tasks.execute(bomb_engine::ExecutionPolicy::MultiThreaded);

    // we will need ways to configure the app to be used as an editor...
    // right now it's just fine to use sample code to get major functionality going.
    // (no point in building an editor for an engine that doesn't exist yet)

    app.loop();

    return 0;
}
// editor.cpp : Defines the entry point for the application.
// right now it acts more like a sandbox to put everything together and test the functionalities.

#include "app/app.h"
#include "pybomb_cpp.h"
#include "tools/coroutine.h"
#include "tools/task_graph.h"

constexpr bool debug = _DEBUG;

MakeCategory(Editor);

auto coroutine() -> bomb_engine::Coroutine
{
    Log(LogTempCategory, LogSeverity::Display, "Coroutine started");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    Log(LogTempCategory, LogSeverity::Display, "Coroutine paused");
    co_await std::suspend_always{};
    Log(LogTempCategory, LogSeverity::Display, "Coroutine resumed");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    Log(LogTempCategory, LogSeverity::Display, "Coroutine ended");
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
    const auto task5 = tasks.add_task(coroutine());

    task1.before({task2, task3});
    task4.after({task2, task1});

    tasks.execute(bomb_engine::ExecutionPolicy::MultiThreaded);

    // we will need ways to configure the app to be used as an editor...
    // right now it's just fine to use sample code to get major functionality going.
    // (no point in building an editor for an engine that doesn't exist yet)
    Log(EditorCategory, LogSeverity::Display, "This is Display");

    app.loop();

    return 0;
}
// editor.cpp : Defines the entry point for the application.
// right now it acts more like a sandbox to put everything together and test the functionalities.

#include <future>

#include "app/app.h"
#include "pybomb_cpp.h"
#include "tools/coroutine.h"
#include "tools/task_graph.h"

constexpr bool debug = _DEBUG;

MakeCategory(Editor);

auto main() -> int
{
    auto app = bomb_engine::App(debug);
    app.start();

    // we will need ways to configure the app to be used as an editor...
    // right now it's just fine to use sample code to get major functionality going.
    // (no point in building an editor for an engine that doesn't exist yet)

    app.loop();

    return 0;
}
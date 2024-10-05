// editor.cpp : Defines the entry point for the application.
// right now it acts more like a sandbox to put everything together and test the functionalities.

#include "app/app.h"
#include "pybomb.h"

#ifdef _DEBUG
constexpr bool debug = true;
#else
constexpr bool debug = false;
#endif

MakeCategory(Editor);

auto main() -> int
{
    auto app = bomb_engine::App(debug);
    app.start();

    auto& py = get_interpreter();

    // we will need ways to configure the app to be used as an editor...
    // right now it's just fine to use sample code to get major functionality going.
    // (no point in building an editor for an engine that doesn't exist yet)
    Log(EditorCategory, LogSeverity::Display, "This is Display");

    app.loop();

    return 0;
}
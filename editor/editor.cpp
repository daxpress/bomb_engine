// editor.cpp : Defines the entry point for the application.
// right now it acts more like a sandbox to put everything together and test the functionalities.

#include "app/app.h"

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

    // we will need ways to configure the app to be used as an editor...
    // right now it's just fine to use sample code to get major functionality going.
    // (no point in building an editor for an engine that doesn't exist yet)
    Log(EditorCategory, LogSeverity::Display, "This is Display");
    Log(EditorCategory, LogSeverity::Log, "This is Log");
    Log(EditorCategory, LogSeverity::Warning, "This is Warning");
    Log(EditorCategory, LogSeverity::Error, "This is Error");
    //Log(EditorCategory, LogSeverity::Fatal, "This is Fatal");

    app.loop();

    return 0;
}
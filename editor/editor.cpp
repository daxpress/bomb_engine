// editor.cpp : Defines the entry point for the application.
// right now it acts more like a sandbox to put everything together and test the functionalities.

#include "app/app.h"

#ifdef _DEBUG
constexpr bool debug = true;
#else
constexpr bool debug = false;
#endif

void test(int, bool) { std::cout << "Test!" << std::endl; }

struct Some
{
    void some(int, bool) { std::cout << "Some!" << std::endl; };
};

auto main() -> int
{
    auto app = bomb_engine::App(debug);

    auto some = Some();

    bomb_engine::Dispatcher<int, bool> dispatcher;
    dispatcher._add_listener([](int, bool) {}, "somelambda");
    dispatcher._add_listener(::test, "::test");
    dispatcher._add_listener(&Some::some, "&Some::some", &some);
    dispatcher(1, true);
    dispatcher.remove_listener(::test);

    app.start();

    // we will need ways to configure the app to be used as an editor...
    // right now it's just fine to use sample code to get major functionality going.
    // (no point in building an editor for an engine that doesn't exist yet)

    app.loop();

    return 0;
}

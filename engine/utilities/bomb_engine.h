#pragma once

// not needed anymore because everything is linked statically (makes sense with an engine)
// keeping it just in case I change my mind

#ifdef _WIN32
#define BOMB_ENGINE_API __declspec(dllexport)
#else
#define BOMB_ENGINE_API
#endif

#pragma once

#ifdef _WIN32
#define BOMB_ENGINE_API __declspec(dllexport)
#else
#define BOMB_ENGINE_API
#endif

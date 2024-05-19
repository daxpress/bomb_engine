// COMMON PRECOMPILED HEADER HEADERS LIST.
// 
//  add here STL stuff 

//structures
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <array>
#include <list>
// other
#include <string>
#include <memory>
#include <cstdint>
#include <cinttypes>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cmath>
#include <optional>
#include <functional>
#include <expected>
#include <filesystem>

// macros

#define BE_NAMESPACE bomb_engine

// mark a class, struct or function with this special macro to tell to expose it 
// to a scripting system (maybe reflection in future).
// can specify an access specifier to tell down to what accessibility level you want to expose:
// public => exposes only public members and methods
// protected => exposes public and protected members and methods
// private => exposes all members and methods
//  Note: it is ignored for a function
#define expose(access_specifier)
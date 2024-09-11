#pragma once

#include <type_traits>
// namespace std

namespace BE_NAMESPACE
{

template <typename... Args>
class Dispatcher
{
    using internal_fn = std::function<void(Args...)>;
    using listeners_map_t = std::unordered_map<size_t, internal_fn>;

public:
    inline void _add_listener(internal_fn&& listener, const std::string&& func_name)
    {
        listeners.emplace(std::hash<std::string>{}(func_name), listener);
    }

    inline void _remove_listener(const std::string&& func_name)
    {
        listeners.erase(std::hash<std::string>{}(func_name));
    }

    template <typename Callable, typename Context>
        requires std::is_member_function_pointer_v<Callable>
    inline void _add_listener(
        Callable callable, const std::string&& func_name, Context* context = nullptr
    )
    {
        auto func = [context, callable](Args... args) { (context->*callable)(args...); };
        _add_listener(func, std::move(func_name));
    }

    void operator()(Args&&... args)
    {
        for (const auto& [key, listener] : listeners)
        {
            listener(args...);
        }
    }

private:
    listeners_map_t listeners{};
};
}  // namespace BE_NAMESPACE

#define add_listener(listener, context) \
    _add_listener<decltype(listener), decltype(context)>(listener, #listener, context)
#define remove_listener(listener) _remove_listener(#listener)

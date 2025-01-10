#pragma once

#include <functional>
#include <mutex>
#include <type_traits>

namespace BE_NAMESPACE
{

template <typename... Args>
class Dispatcher
{
private:
    using internal_fn = std::function<void(Args...)>;
    using listeners_map_t = std::unordered_map<size_t, internal_fn>;

    // uuid generation overloads for the method

    template <typename Callable, typename Context>
        requires std::is_member_function_pointer_v<Callable>
    static auto generate_key(Callable callable, Context* context) -> size_t
    {
        // Combine the hash of the object pointer with the hash of the callable
        // this allows to have same methods for different objects in the same dispatcher
        return std::hash<Context*>()(context) ^ typeid(callable).hash_code();
    }

    // uuid generation overloads for the function

    template <typename Callable>
        requires(!std::is_member_function_pointer_v<Callable>)
    static auto generate_key(Callable callable) -> size_t
    {
        // it doesn't make sense to put the same free function twice in a dispatcher so this is fine
        // to me
        return typeid(callable).hash_code();
    }

    // common method for placing the listener in the map
    inline void place_listener(const size_t key, internal_fn&& fn)
    {
        std::lock_guard lock(mutex);
        listeners.emplace(key, std::move(fn));
    }

public:
    // add_listener overloads for functions and methods and restrict compilation using requirements
    template <typename Callable>
        requires(!std::is_member_function_pointer_v<Callable>)
    inline void add_listener(Callable&& listener)
    {
        place_listener(generate_key(listener), std::forward<Callable>(listener));
    }

    // only compiled when Callable is a member function (method)

    template <typename Callable, typename Context>
        requires std::is_member_function_pointer_v<Callable>
    inline void add_listener(Callable callable, Context* context)
    {
        auto uuid = generate_key<Callable, Context>(callable, context);
        auto func = [context, callable](Args... args) { (context->*callable)(args...); };
        place_listener(uuid, func);
    }

    // method removal

    template <typename Callable, typename Context>
        requires std::is_member_function_pointer_v<Callable>
    inline void remove_listener(Callable callable, Context* context)
    {
        std::lock_guard lock(mutex);
        listeners.erase(generate_key<Callable, Context>(callable, context));
    }

    template <typename Callable>
        requires(!std::is_member_function_pointer_v<Callable>)
    inline void remove_listener(Callable callable)
    {
        std::lock_guard lock(mutex);
        listeners.erase(generate_key(callable));
    }

    // dispatch

    inline void operator()(Args&&... args) { broadcast(std::forward<Args>(args)...); }

    // for those who prefer explicitness

    inline void broadcast(Args&&... args)
    {
        std::lock_guard lock(mutex);
        for (const auto& [key, listener] : listeners)
        {
            listener(args...);
        };
    }

private:
    listeners_map_t listeners{};
    std::mutex mutex{};
};
}  // namespace BE_NAMESPACE

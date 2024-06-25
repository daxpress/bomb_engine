#pragma once

#include <entt/entt.hpp>

namespace BE_NAMESPACE
{
// Based on entt poly crash course => allows to query different Scriptables using Scriptable in the
// view creation (I hope...)

// Internal definition for the entt::poly wrapper, use Scriptable instead
struct __Scriptable : entt::type_list<>
{
    // inherits from entt::type_list<> to allow deducibility

    template <typename Base>
    struct type : Base
    {
        // the "concept" defines the methods we want to implement
        // an alternative to entt::poly_call is this->template invoke<>, prefer the first one
        void start() { entt::poly_call<0>(*this); }
        void update(float tick) { entt::poly_call<1>(*this, tick); }
        // they can also take other args and return different values...
        // and the methods for the concept simply invoke the actual implementation passing the
        // arguments
    };

    // this defines how the concept is fulfilled
    template <typename Type>
    using impl = entt::value_list<&Type::start, &Type::update>;
};

// let's be honest, I don't really grasp most of this template magic...

// at this point use entt::poly to wrap the compatible classes; this will be the class used to add
// the component
using Scriptable = entt::poly<__Scriptable>;
}  // namespace BE_NAMESPACE
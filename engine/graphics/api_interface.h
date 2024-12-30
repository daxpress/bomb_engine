#pragma once

#include "e_api.h"

namespace BE_NAMESPACE
{
class IAPI
{
public:
    virtual ~IAPI() = default;

    inline virtual auto get_api() -> E_API = 0;

    virtual void draw_frame() = 0;
};
}  // namespace BE_NAMESPACE
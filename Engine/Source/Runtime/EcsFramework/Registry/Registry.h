#pragma once

#include "Runtime/EcsFramework/Component/ComponentGroup.h"

#include <entt.hpp>

namespace XLEngine
{
    class Registry final
    {
    public:
        Registry() = default;
        ~Registry() = default;

    private:
        entt::registry mRegistry;
    };
}
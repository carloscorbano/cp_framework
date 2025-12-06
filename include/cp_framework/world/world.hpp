#pragma once

#include <entt/entt.hpp>
#include "cp_framework/core/export.hpp"
#include "cp_framework/core/types.hpp"

namespace cp
{
    class CP_API World
    {
    public:
        World();
        ~World();

        CP_RULE_OF_FIVE_DELETE(World);

    private:
        UPTR<entt::registry> m_registry;
    };
} // namespace cp

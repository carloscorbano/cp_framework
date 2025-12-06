#include "cp_framework/world/world.hpp"

namespace cp
{
    World::World()
    {
        m_registry = M_UPTR<entt::registry>();
    }

    World::~World()
    {
    }
}
#pragma once

#include "cp_framework/core/export.hpp"
#include "cp_framework/core/types.hpp"
#include "hybridEvents.hpp"

namespace cp
{
    /**
     * @class EventSystem
     * @brief Provides global access to a singleton instance of the HybridEventDispatcher.
     *
     * This class acts as a centralized event system, exposing a static method
     * that returns a unique instance of the HybridEventDispatcher. It ensures
     * that only one dispatcher exists throughout the application.
     */
    class EventSystem
    {
    public:
        MAKE_SINGLETON(HybridEventDispatcher);
    };
} // namespace cp

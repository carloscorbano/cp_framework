#pragma once

#include "cp_framework/core/export.hpp"
#include "cp_framework/core/hybridEvents.hpp"

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
    class CP_API EventSystem
    {
    public:
        /**
         * @brief Returns the global HybridEventDispatcher instance.
         *
         * This method creates the dispatcher on the first call and returns
         * a reference to the same instance on all subsequent calls.
         *
         * @return Reference to the singleton HybridEventDispatcher.
         */
        static HybridEventDispatcher &Get()
        {
            static HybridEventDispatcher instance;
            return instance;
        }
    };
} // namespace cp

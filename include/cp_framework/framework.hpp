#pragma once

#include "core/export.hpp"
#include "core/types.hpp"
#include <atomic>

namespace cp
{
    //-------------------------
    // FORWARD DECLARATIONS
    //-------------------------
    class Window;

    /**
     * @brief Framework class, this class controll the framework creation and game loop
     */
    class CP_API Framework
    {
    public:
        Framework();
        ~Framework();

        CP_RULE_OF_FIVE_DELETE(Framework);

        /**
         * @brief Initializes the framework.
         */
        void Init();

        /**
         * @brief Starts the main game loop.
         */
        void Run();

    private:
        bool m_initializated{false};
        std::atomic<bool> m_isRunning{false};

        UPTR<Window> m_window;
    };
} // namespace cp

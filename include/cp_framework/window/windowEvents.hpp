#pragma once

#include "cp_framework/events/events.hpp"

namespace cp
{
    struct onWindowSizeChangedEvent : public Event
    {
        int newWidth;
        int newHeight;
    };

    struct onWindowFocusedEvent : public Event
    {
        bool focused;
    };

} // namespace cp

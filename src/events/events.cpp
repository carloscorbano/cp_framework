#include "cp_framework/events/events.hpp"

namespace cp
{
    EventDispatcher::EventDispatcher() : m_nextListenerID(1)
    {
        StartAsync();
    }

    EventDispatcher::~EventDispatcher()
    {
        StopAsync();
    }

    void EventDispatcher::StartAsync()
    {
        m_running = true;
        m_thread = std::thread([this]()
                               { ProcessQueue(); });

        m_cv.notify_one();
    }

    void EventDispatcher::StopAsync()
    {
        if (m_running)
        {
            m_running = false;
            m_cv.notify_one();
            if (m_thread.joinable())
                m_thread.join();
        }
    }

    void EventDispatcher::ProcessQueue()
    {
        while (m_running)
        {
            std::shared_ptr<EventWrapper> ev;
            {
                std::unique_lock lock(m_queueMutex);
                m_cv.wait(lock, [this]()
                          { return !m_eventQueue.empty() || !m_running; });
                if (!m_running)
                    break;

                ev = m_eventQueue.front();
                m_eventQueue.pop();
            }

            if (ev)
                ev->Dispatch(this);
        }
    }
}
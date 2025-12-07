/**
 * @file EventDispatcher.hpp
 * @brief Event dispatching system supporting synchronous and asynchronous dispatch with listener priorities.
 */

#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <queue>
#include <optional>
#include <thread>
#include <condition_variable>
#include "cp_framework/core/export.hpp"

namespace cp
{

    /**
     * @brief Base class for all events.
     *
     * All event types must derive from this class.
     */
    struct Event
    {
        virtual ~Event() = default;
    };

    /**
     * @brief Unique identifier for listeners.
     */
    using ListenerID = uint64_t;

    /**
     * @brief Manages registration, dispatching, and asynchronous queuing of events.
     *
     * Supports:
     * - Listener registration with priority
     * - Listener removal
     * - Immediate (synchronous) event dispatch
     * - Asynchronous event queuing with thread processing
     */
    class EventDispatcher
    {
    public:
        /**
         * @brief Constructs the dispatcher and initializes the listener ID counter.
         */
        EventDispatcher();

        /**
         * @brief Destructor. Ensures that the async processing thread is stopped safely.
         */
        ~EventDispatcher();

        /**
         * @brief Subscribes a listener to a specific event type.
         *
         * @tparam EventType The event type to subscribe to.
         * @param callback Function to be called when the event is emitted.
         * @param priority Higher priority listeners are called earlier.
         * @return ListenerID A unique ID that can be used to unsubscribe.
         */
        template <typename EventType>
        ListenerID Subscribe(std::function<void(const EventType &)> callback, int priority = 0)
        {
            const std::type_index type = typeid(EventType);
            std::unique_lock lock(m_mutex);

            ListenerID id = m_nextListenerID++;
            auto wrapper = [callback](const Event &e)
            {
                callback(static_cast<const EventType &>(e));
            };

            ListenerEntry entry{id, priority, wrapper};

            auto &vec = m_listeners[type];
            vec.push_back(entry);

            // Sort by priority (descending)
            std::sort(vec.begin(), vec.end(),
                      [](const ListenerEntry &a, const ListenerEntry &b)
                      {
                          return a.priority > b.priority;
                      });

            return id;
        }

        /**
         * @brief Unsubscribes a previously registered listener.
         *
         * @tparam EventType The event type associated with the listener.
         * @param id The listener ID returned from Subscribe().
         */
        template <typename EventType>
        void Unsubscribe(ListenerID id)
        {
            const std::type_index type = typeid(EventType);
            std::unique_lock lock(m_mutex);

            auto it = m_listeners.find(type);
            if (it == m_listeners.end())
                return;

            auto &vec = it->second;
            vec.erase(std::remove_if(vec.begin(), vec.end(),
                                     [id](const ListenerEntry &e)
                                     { return e.id == id; }),
                      vec.end());
        }

        /**
         * @brief Emits an event immediately (synchronously).
         *
         * @tparam EventType The event type to emit.
         * @param event The event instance.
         */
        template <typename EventType>
        void Emit(const EventType &event)
        {
            const std::type_index type = typeid(EventType);
            std::unique_lock lock(m_mutex);

            auto it = m_listeners.find(type);
            if (it == m_listeners.end())
                return;

            for (auto &entry : it->second)
                entry.callback(event);
        }

        // ===========================================================
        //   ASYNCHRONOUS EVENT SUPPORT
        // ===========================================================

        /**
         * @brief Queues an event for asynchronous processing.
         *
         * @tparam EventType The event type.
         * @param event The event instance to enqueue.
         */
        template <typename EventType>
        void QueueEvent(const EventType &event)
        {
            std::unique_lock lock(m_queueMutex);
            m_eventQueue.push(std::make_shared<EventWrapperTyped<EventType>>(event));
            m_cv.notify_one();
        }

        /**
         * @brief Starts the asynchronous event processing thread.
         */
        void StartAsync();

        /**
         * @brief Stops the asynchronous event thread safely.
         *
         * Wakes the thread if blocked and waits for it to join.
         */
        void StopAsync();

    private:
        /**
         * @brief Represents a registered listener.
         */
        struct ListenerEntry
        {
            ListenerID id;                               ///< Unique listener ID
            int priority;                                ///< Listener priority
            std::function<void(const Event &)> callback; ///< Callback function
        };

        std::unordered_map<std::type_index, std::vector<ListenerEntry>> m_listeners; ///< Listeners indexed by event type
        std::mutex m_mutex;                                                          ///< Mutex for listener map
        std::atomic<ListenerID> m_nextListenerID;                                    ///< Generates unique listener IDs

        /**
         * @brief Abstract wrapper for queued events.
         *
         * Enables storing events of different types in the same queue.
         */
        struct EventWrapper
        {
            virtual ~EventWrapper() = default;

            /**
             * @brief Dispatches the stored event through the EventDispatcher.
             */
            virtual void Dispatch(EventDispatcher *dispatcher) = 0;
        };

        /**
         * @brief Templated wrapper that holds a specific event type.
         *
         * @tparam T The event type.
         */
        template <typename T>
        struct EventWrapperTyped : EventWrapper
        {
            T event; ///< Stored event instance

            EventWrapperTyped(const T &e) : event(e) {}

            void Dispatch(EventDispatcher *dispatcher) override
            {
                dispatcher->Emit(event);
            }
        };

        std::queue<std::shared_ptr<EventWrapper>> m_eventQueue; ///< Queue of pending events
        std::mutex m_queueMutex;                                ///< Mutex protecting the event queue
        std::condition_variable m_cv;                           ///< Condition variable for queue notifications
        std::thread m_thread;                                   ///< Thread processing asynchronous events
        std::atomic<bool> m_running{false};                     ///< Indicates whether the thread is running

        /**
         * @brief Internal loop executed by the asynchronous processing thread.
         *
         * Waits for queued events and dispatches them.
         */
        void ProcessQueue();
    };
} // namespace cp

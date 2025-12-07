#pragma once

#include "events.hpp"
#include "delegate.hpp"
#include <type_traits>

namespace cp
{
    // =======================================
    // HybridEventDispatcher
    // Combines asynchronous EventDispatcher with Delegates
    // =======================================

    /**
     * @class HybridEventDispatcher
     * @brief A hybrid event dispatcher that combines the asynchronous event
     *        system of EventDispatcher with Delegate-based callbacks.
     *
     * This class allows listeners to be registered using Delegates, lambdas,
     * functors, or any callable type. It supports both synchronous event
     * emission and asynchronous event queuing through the inherited
     * EventDispatcher functionality.
     */
    class HybridEventDispatcher : public EventDispatcher
    {
    public:
        /**
         * @brief Default constructor.
         */
        CP_API HybridEventDispatcher() = default;

        /**
         * @brief Default destructor.
         */
        CP_API ~HybridEventDispatcher() = default;

        // ---------------------------
        // Subscribe using Delegate
        // ---------------------------

        /**
         * @brief Subscribes a listener using a Delegate.
         *
         * @tparam EventType The type of event to listen for.
         * @param del The delegate to invoke when the event is emitted.
         * @param priority Listener priority (higher = executed earlier).
         * @return ListenerID A unique ID representing the registered listener.
         */
        template <typename EventType>
        CP_API_EXPORT ListenerID Subscribe(const Delegate<void(const EventType &)> &del, int priority = 0)
        {
            // Capture delegate in local variable
            Delegate<void(const EventType &)> localDel = del;

            return this->EventDispatcher::template Subscribe<EventType>(
                [localDel](const EventType &e)
                {
                    localDel.Invoke(e);
                },
                priority);
        }

        // ---------------------------
        // Subscribe using any callable
        // ---------------------------

        /**
         * @brief Subscribes a listener using any callable object
         *        (lambda, function, functor, std::function, etc.).
         *
         * @tparam EventType The type of event to listen for.
         * @tparam F The type of the callable.
         * @param callback The callable that will be invoked on event emission.
         * @param priority Listener priority.
         * @return ListenerID A unique ID representing the registered listener.
         */
        template <typename EventType, typename F>
        CP_API_EXPORT ListenerID Subscribe(F &&callback, int priority = 0)
        {
            return this->EventDispatcher::template Subscribe<EventType>(
                std::forward<F>(callback), priority);
        }

        // ---------------------------
        // Remove listener
        // ---------------------------

        /**
         * @brief Unsubscribes a listener from a specific event type.
         *
         * @tparam EventType The type of event.
         * @param id The listener ID obtained from Subscribe().
         */
        template <typename EventType>
        CP_API_EXPORT void Unsubscribe(ListenerID id)
        {
            this->EventDispatcher::Unsubscribe<EventType>(id);
        }

        // ---------------------------
        // Emit (synchronous)
        // ---------------------------

        /**
         * @brief Emits an event immediately (synchronously).
         *
         * @tparam EventType The type of event.
         * @param e The event instance.
         */
        template <typename EventType>
        CP_API_EXPORT void Emit(const EventType &e)
        {
            this->EventDispatcher::Emit<EventType>(e);
        }

        // ---------------------------
        // QueueEvent (asynchronous)
        // ---------------------------

        /**
         * @brief Queues an event for asynchronous processing.
         *
         * @tparam EventType The type of event.
         * @param e The event instance.
         */
        template <typename EventType>
        CP_API_EXPORT void QueueEvent(const EventType &e)
        {
            this->EventDispatcher::QueueEvent<EventType>(e);
        }

        // ---------------------------
        // Start/Stop asynchronous thread
        // ---------------------------

        /**
         * @brief Starts the internal asynchronous event thread.
         */
        void StartAsync() { EventDispatcher::StartAsync(); }

        /**
         * @brief Stops the internal asynchronous event thread.
         */
        void StopAsync() { EventDispatcher::StopAsync(); }
    };
} // namespace cp

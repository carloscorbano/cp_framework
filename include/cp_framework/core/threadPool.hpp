#pragma once

#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <atomic>
#include <random>
#include <memory>
#include "export.hpp"

namespace cp
{
    /**
     * @enum TaskPriority
     * @brief Defines scheduling priority for tasks submitted to the ThreadPool.
     *
     * - HIGH   → Task is inserted at the front of the queue.
     * - NORMAL → Task is appended at the back of the queue.
     * - LOW    → Behaves like NORMAL (unless custom logic is added).
     *
     * @ingroup Threading
     */
    enum class CP_API TaskPriority
    {
        HIGH,
        NORMAL,
        LOW
    };

    /**
     * @class ThreadPool
     * @brief A multithreaded task scheduler with priority queues and work distribution.
     *
     * Features:
     * - Fixed-size pool of worker threads.
     * - Fair distribution of tasks across threads (randomized index selection).
     * - Support for prioritized scheduling (high-priority tasks go to the front).
     * - Thread-safe job submission.
     * - Graceful shutdown via Shutdown().
     *
     * Internally, each worker thread has:
     * - A dedicated task queue,
     * - A mutex for synchronization,
     * - A condition variable for sleeping and waking.
     *
     * @ingroup Threading
     */
    class CP_API ThreadPool
    {
    public:
        /**
         * @brief Constructs a thread pool with the specified number of worker threads.
         *
         * @param threadCount Number of worker threads to spawn.
         *                    Defaults to std::thread::hardware_concurrency().
         */
        explicit ThreadPool(size_t threadCount = std::thread::hardware_concurrency());

        /**
         * @brief Destructs the thread pool and shuts down all workers.
         *
         * Equivalent to calling Shutdown().
         */
        ~ThreadPool();

        ThreadPool(const ThreadPool &) = delete;
        ThreadPool &operator=(const ThreadPool &) = delete;
        ThreadPool(ThreadPool &&) = delete;
        ThreadPool &operator=(ThreadPool &&) = delete;

        /**
         * @brief Submits a callable task for asynchronous execution.
         *
         * The task is wrapped into a `std::packaged_task`, stored in one of the
         * internal task queues (chosen randomly) and executed by a worker thread.
         *
         * Priority behavior:
         * - HIGH priority → task is pushed to the front of the queue.
         * - NORMAL / LOW  → task is appended to the back of the queue.
         *
         * @tparam Func Callable type.
         * @tparam Args Argument pack to forward to the function.
         *
         * @param priority Scheduling priority for this task.
         * @param f Function/callable to invoke.
         * @param args Arguments forwarded to the callable.
         *
         * @return std::future<ReturnType> A future holding the callable’s return value.
         *
         * @throws std::runtime_error if the pool is no longer running.
         *
         * @ingroup Threading
         */
        template <typename Func, typename... Args>
        auto Submit(TaskPriority priority, Func &&f, Args &&...args)
            -> std::future<decltype(f(args...))>;

        /**
         * @brief Signals all workers to stop and waits for them to finish.
         *
         * After Shutdown() is called, no more tasks can be submitted.
         */
        void Shutdown();

    private:
        /**
         * @brief Main loop executed by each worker thread.
         *
         * Continuously waits for tasks, processes them, and exits when the pool is shut down.
         *
         * @param index Index of the worker thread and its associated queue.
         */
        void WorkerLoop(size_t index);

    private:
        std::vector<std::deque<std::function<void()>>> m_queues; ///< Per-thread task queues.
        std::vector<std::mutex> m_mutexes;                       ///< One mutex per queue.
        std::vector<std::condition_variable> m_conditions;       ///< One condition variable per queue.
        std::vector<std::thread> m_workers;                      ///< Worker thread handles.
        std::atomic_bool m_running;                              ///< Indicates whether the pool accepts tasks.

        std::mt19937 m_rng{std::random_device{}()};   ///< RNG for queue selection.
        std::uniform_int_distribution<size_t> m_dist; ///< Distribution over worker index range.
    };

    // ---------------- Template Implementation ----------------

    /**
     * @brief Template implementation for task submission.
     *
     * Wraps the callable in a packaged_task, selects a queue randomly,
     * and inserts the task respecting the provided priority.
     *
     * @see Submit()
     */
    template <typename Func, typename... Args>
    auto ThreadPool::Submit(TaskPriority priority, Func &&f, Args &&...args)
        -> std::future<decltype(f(args...))>
    {
        using ReturnType = decltype(f(args...));

        if (!m_running.load(std::memory_order_acquire))
            throw std::runtime_error("ThreadPool is shut down");

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<Func>(f), std::forward<Args>(args)...));
        std::future<ReturnType> future = task->get_future();

        size_t idx = m_dist(m_rng);
        {
            std::lock_guard<std::mutex> lock(m_mutexes[idx]);
            if (priority == TaskPriority::HIGH)
                m_queues[idx].emplace_front([task]
                                            { (*task)(); });
            else
                m_queues[idx].emplace_back([task]
                                           { (*task)(); });
        }

        m_conditions[idx].notify_one();
        return future;
    }
}
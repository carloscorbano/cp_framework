/**
 * @file Delegate.hpp
 * @brief Provides Delegate and MulticastDelegate classes, supporting binding of free functions,
 * lambdas, instance methods, and const methods. Includes priority-based multicast support.
 */

#pragma once

#include <functional>
#include <mutex>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <type_traits>
#include "export.hpp"

#ifdef _DEBUG
#include "debug.hpp"
#define CP_DELEGATE_LOG(...) LOG_INFO(__VA_ARGS__)
#define CP_DELEGATE_LOG_DEBUG(...) LOG_DEBUG(__VA_ARGS__)
#define CP_DELEGATE_LOG_WARN(...) LOG_WARN(__VA_ARGS__)
#else
#define CP_DELEGATE_LOG(...) (void)0
#define CP_DELEGATE_LOG_DEBUG(...) (void)0
#define CP_DELEGATE_LOG_WARN(...) (void)0
#endif

namespace cp
{

    /**
     * @brief Forward declaration of Delegate template.
     */
    template <typename Signature>
    class Delegate;

    /**
     * @brief Delegate implementation for free functions, lambdas, and member functions.
     *
     * @tparam R Return type.
     * @tparam Args Parameter pack.
     *
     * This class stores callable objects or bound member functions. It supports:
     * - Binding lambdas and `std::function`
     * - Binding instance methods (const and non-const)
     * - Equality check based on method ID and instance pointer
     */
    template <typename R, typename... Args>
    class CP_API Delegate<R(Args...)>
    {
    public:
        using FuncType = std::function<R(Args...)>;

        /**
         * @brief Default constructor (creates empty delegate).
         */
        Delegate() = default;

        /**
         * @brief Constructs a delegate from a free function or compatible callable.
         * @param f Callable object.
         */
        Delegate(FuncType f) : func_(std::move(f)) {}

        /**
         * @brief Factory that creates a Delegate object from a std::function.
         *
         * @tparam T Function signature.
         * @param func Function to bind.
         * @return A new Delegate instance.
         */
        template <typename T>
        static Delegate<T> FromFunction(const std::function<T> &func)
        {
            Delegate<T> del;
            del.SetFunction(func);
            return del;
        }

        /**
         * @brief Factory for wrapping lambdas or std::function into a Delegate.
         */
        static Delegate FromLambda(FuncType f)
        {
            return Delegate(std::move(f));
        }

        /**
         * @brief Binds a lambda or callable object.
         *
         * @tparam F Callable type.
         * @param f Callable object.
         */
        template <typename F>
        void Bind(F &&f)
        {
            func_ = std::forward<F>(f);
            instance_ptr_ = nullptr;
            method_id_ = 0;
            CP_DELEGATE_LOG_DEBUG("[Delegate] Bound Lambda/Callable");
        }

        /**
         * @brief Binds a non-const instance method.
         *
         * @tparam T Class type.
         * @param instance Pointer to object instance.
         * @param method Member function pointer.
         */
        template <typename T>
        void Bind(T *instance, R (T::*method)(Args...))
        {
            func_ = [instance, method](Args... args) -> R
            {
                return (instance->*method)(std::forward<Args>(args)...);
            };
            instance_ptr_ = instance;
            method_id_ = MethodId(method);
            CP_DELEGATE_LOG_DEBUG("[Delegate] Bound Method -> instance={} method={}", (void *)instance, typeid(method).name());
        }

        /**
         * @brief Binds a const instance method.
         *
         * @tparam T Class type.
         * @param instance Pointer to object instance.
         * @param method Const member function pointer.
         */
        template <typename T>
        void Bind(const T *instance, R (T::*method)(Args...) const)
        {
            func_ = [instance, method](Args... args) -> R
            {
                return (instance->*method)(std::forward<Args>(args)...);
            };
            instance_ptr_ = const_cast<T *>(instance);
            method_id_ = MethodId(method);
            CP_DELEGATE_LOG_DEBUG("[Delegate] Bound Const Method -> instance={} method={}", (void *)instance, typeid(method).name());
        }

        /**
         * @brief Clears the delegate, removing the bound function or method.
         */
        void Unbind()
        {
            func_ = nullptr;
            instance_ptr_ = nullptr;
            method_id_ = 0;
            CP_DELEGATE_LOG_DEBUG("[Delegate] Unbind");
        }

        /**
         * @brief Checks whether the delegate has a bound callable.
         * @return True if empty.
         */
        bool Empty() const { return !static_cast<bool>(func_); }

        /**
         * @brief Function call operator.
         */
        void operator()(Args... args) const { Invoke(std::forward<Args>(args)...); }

        /**
         * @brief Invokes the bound function or method.
         *
         * If the return type is void, no value is returned.
         */
        R Invoke(Args... args) const
        {
            if (func_)
            {
                if constexpr (std::is_void_v<R>)
                    func_(std::forward<Args>(args)...);
                else
                    return func_(std::forward<Args>(args)...);
            }
        }

        /**
         * @brief Equality operator.
         *
         * Only delegates bound to the same instance AND same method are equal.
         */
        bool operator==(const Delegate &other) const
        {
            return instance_ptr_ == other.instance_ptr_ && method_id_ == other.method_id_;
        }

    private:
        /**
         * @brief Produces a unique identifier for a member function pointer.
         *
         * @tparam T Class type.
         * @tparam M Member pointer type.
         * @param method Member function pointer.
         */
        template <typename T, typename M>
        static size_t MethodId(M T::*method)
        {
            return reinterpret_cast<size_t>(*(void **)&method);
        }

        FuncType func_;                ///< Stored callable
        void *instance_ptr_ = nullptr; ///< Instance pointer for method binding
        size_t method_id_ = 0;         ///< Unique method ID for comparison
    };

    // ========================================================
    // MULTICAST DELEGATE
    // ========================================================

    /**
     * @brief Multicast delegate: stores multiple delegates and calls them in sorted priority order.
     *
     * @tparam Signature Function signature (R(Args...)).
     */
    template <typename Signature>
    class MulticastDelegate;

    /**
     * @brief Multicast delegate implementation.
     *
     * Supports:
     * - Adding delegates with priority
     * - Removing delegates
     * - Calling all delegates in priority order
     * - Tracking call counts for each delegate
     */
    template <typename R, typename... Args>
    class CP_API MulticastDelegate<R(Args...)>
    {
    public:
        using DelegateType = Delegate<R(Args...)>;

        /**
         * @brief Represents a single delegate entry in the multicast list.
         */
        struct Entry
        {
            DelegateType delegate;  ///< Stored delegate
            int32_t priority = 0;   ///< Higher priority delegates are called first
            uint64_t callCount = 0; ///< Number of times this delegate was invoked
        };

        /**
         * @brief Adds an existing delegate to the multicast list.
         *
         * @param del Delegate to add.
         * @param priority Priority value.
         */
        void Add(const DelegateType &del, int32_t priority = 0)
        {
            std::scoped_lock lock(mutex_);
            entries_.push_back({del, priority, 0});
            SortEntries();
            CP_DELEGATE_LOG("[MulticastDelegate] Added delegate -> total={}, priority={}", entries_.size(), priority);
        }

        /**
         * @brief Adds a lambda or callable object.
         */
        template <typename F, typename = std::enable_if_t<!std::is_same_v<std::decay_t<F>, DelegateType>>>
        void Add(F &&f, int32_t priority = 0)
        {
            DelegateType del;
            del.Bind(std::forward<F>(f));
            Add(del, priority);
        }

        /**
         * @brief Adds a non-const instance method.
         */
        template <typename T>
        void Add(T *instance, R (T::*method)(Args...), int32_t priority = 0)
        {
            DelegateType del;
            del.Bind(instance, method);
            Add(del, priority);
        }

        /**
         * @brief Adds a const instance method.
         */
        template <typename T>
        void Add(const T *instance, R (T::*method)(Args...) const, int32_t priority = 0)
        {
            DelegateType del;
            del.Bind(instance, method);
            Add(del, priority);
        }

        /**
         * @brief Removes a specific delegate.
         */
        void Remove(const DelegateType &del)
        {
            std::scoped_lock lock(mutex_);
            size_t before = entries_.size();

            entries_.erase(
                std::remove_if(entries_.begin(), entries_.end(),
                               [&](const Entry &e)
                               { return e.delegate == del; }),
                entries_.end());

            size_t removed = before - entries_.size();
            if (removed > 0)
                CP_DELEGATE_LOG("[MulticastDelegate] Removed {} delegate(s), remaining={}", removed, entries_.size());
        }

        /**
         * @brief Removes a bound non-const instance method.
         */
        template <typename T>
        void Remove(T *instance, R (T::*method)(Args...))
        {
            DelegateType tmp;
            tmp.Bind(instance, method);
            Remove(tmp);
        }

        /**
         * @brief Removes a bound const method.
         */
        template <typename T>
        void Remove(const T *instance, R (T::*method)(Args...) const)
        {
            DelegateType tmp;
            tmp.Bind(instance, method);
            Remove(tmp);
        }

        /**
         * @brief Clears all delegates.
         */
        void Clear()
        {
            std::scoped_lock lock(mutex_);
            CP_DELEGATE_LOG("[MulticastDelegate] Clearing all delegates -> total before clear = {}", entries_.size());
            entries_.clear();
        }

        /**
         * @brief Checks if no delegates are stored.
         */
        bool Empty() const
        {
            std::scoped_lock lock(mutex_);
            return entries_.empty();
        }

        /**
         * @brief Invokes all stored delegates in priority order.
         */
        void operator()(Args... args)
        {
            std::scoped_lock lock(mutex_);
            const size_t total = entries_.size();

            if (total == 0)
            {
                CP_DELEGATE_LOG("=== Emission aborted: no delegates registered ===");
                return;
            }

            CP_DELEGATE_LOG("=== Emitting MulticastDelegate -> total delegates = {} ===", total);
            size_t idx = 1;

            for (auto &e : entries_)
            {
                CP_DELEGATE_LOG_DEBUG("[CALL {}/{}] Invoking delegate", idx, total);

                e.delegate.Invoke(std::forward<Args>(args)...);

                e.callCount++;

                if (e.delegate.Empty())
                {
                    CP_DELEGATE_LOG_WARN("Empty delegate at index {}", idx);
                }

                idx++;
            }

            CP_DELEGATE_LOG("Call counters after emission:");
            idx = 1;
            for (auto &e : entries_)
            {
                CP_DELEGATE_LOG("    [{}] callCount = {}", idx, e.callCount);
                idx++;
            }

            CP_DELEGATE_LOG("=== End of emission ===");
        }

        /**
         * @brief Returns read-only access to stored entries.
         */
        const std::vector<Entry> &GetEntries() const { return entries_; }

    private:
        /**
         * @brief Sorts delegate entries by priority (descending).
         */
        void SortEntries()
        {
            std::sort(entries_.begin(), entries_.end(),
                      [](const Entry &a, const Entry &b)
                      { return a.priority > b.priority; });
        }

        mutable std::mutex mutex_;   ///< Protects access to entries
        std::vector<Entry> entries_; ///< Ordered delegate list
    };

} // namespace cp

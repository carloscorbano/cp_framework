#pragma once

#include <string>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <optional>
#include <memory>
#include <type_traits>
#include <functional>
#include <nlohmann/json.hpp>

/**
 * @defgroup Serialization Serialization System
 * @brief Automatic JSON/BSON serialization utilities for engine objects.
 *
 * This module provides:
 * - Reflection-style field registration
 * - Automatic JSON + BSON serialization
 * - Support for STL containers, optionals, pointers, and nested objects
 *
 * @{
 */

/**
 * @defgroup SerializationCore Core Serialization API
 * @ingroup Serialization
 * @brief Public API for registering fields and serializing/deserializing.
 * @{
 */

namespace cp {

    /**
     * @class SerializableBase
     * @brief Base class providing automatic JSON/BSON serialization for derived types.
     *
     * Any derived object can declare serializable fields through RegisterField(),
     * enabling full reflection-based JSON/BSON serialization using nlohmann::json.
     *
     * Supported field types:
     * - Primitive types
     * - STL containers (vector, array, map, unordered_map)
     * - std::optional
     * - std::unique_ptr
     * - Nested SerializableBase-derived types (recursive serialization)
     *
     * @ingroup SerializationCore
     */
    class SerializableBase {
    public:
        /// Virtual destructor.
        virtual ~SerializableBase() = default;

        // ---------------------------------------------------------------------
        // Field Registration
        // ---------------------------------------------------------------------

        /**
         * @brief Registers a field for serialization and deserialization.
         *
         * @tparam T Type of the field.
         * @param name JSON key associated with the field.
         * @param field Reference to the variable.
         *
         * @ingroup SerializationCore
         */
        template<typename T>
        void RegisterField(const std::string& name, T& field) {
            m_fields[name] = [&field]() -> nlohmann::json {
                return SerializeField(field);
            };
            m_deserializers[name] = [&field](const nlohmann::json& j) {
                DeserializeField(field, j);
            };
        }

        // ---------------------------------------------------------------------
        // JSON Serialization
        // ---------------------------------------------------------------------

        /**
         * @brief Serializes all registered fields into a JSON object.
         * @return JSON object containing all registered fields.
         *
         * @ingroup SerializationCore
         */
        nlohmann::json Serialize() const {
            nlohmann::json j;
            for (auto& [name, getter] : m_fields) {
                j[name] = getter();
            }
            return j;
        }

        /**
         * @brief Serializes the object into BSON (binary JSON format).
         * @return A vector of bytes containing BSON data.
         *
         * @ingroup SerializationCore
         */
        std::vector<uint8_t> SerializeBSON() const {
            return nlohmann::json::to_bson(Serialize());
        }

        // ---------------------------------------------------------------------
        // JSON Deserialization
        // ---------------------------------------------------------------------

        /**
         * @brief Populates registered fields based on a JSON object.
         *
         * Only keys that exist in the provided JSON object will be modified.
         *
         * @param j JSON object containing field data.
         *
         * @ingroup SerializationCore
         */
        void Deserialize(const nlohmann::json& j) {
            for (auto& [name, setter] : m_deserializers) {
                if (j.contains(name)) {
                    setter(j.at(name));
                }
            }
        }

        /**
         * @brief Deserializes from BSON into the object.
         *
         * @param data BSON data previously produced by SerializeBSON().
         *
         * @ingroup SerializationCore
         */
        void DeserializeBSON(const std::vector<uint8_t>& data) {
            Deserialize(nlohmann::json::from_bson(data));
        }

    protected:
        // ---------------------------------------------------------------------
        // Generic Serialization Helpers
        // ---------------------------------------------------------------------

        /**
         * @brief Converts a field into JSON representation.
         *
         * Supports:
         * - SerializableBase (recursive)
         * - STL containers
         * - Optional and unique_ptr types
         * - Primitive types
         *
         * @tparam T Field type.
         * @param value Field value.
         * @return JSON representation.
         *
         * @ingroup SerializationCore
         */
        template<typename T>
        static nlohmann::json SerializeField(const T& value) {
            if constexpr (std::is_base_of<SerializableBase, T>::value) {
                return value.Serialize();
            } else if constexpr (is_vector<T>::value || is_array<T>::value) {
                nlohmann::json arr = nlohmann::json::array();
                for (const auto& el : value) {
                    arr.push_back(SerializeField(el));
                }
                return arr;
            } else if constexpr (is_map<T>::value || is_unordered_map<T>::value) {
                nlohmann::json obj = nlohmann::json::object();
                for (const auto& [k, v] : value) {
                    obj[k] = SerializeField(v);
                }
                return obj;
            } else if constexpr (is_optional<T>::value) {
                return value.has_value() ? SerializeField(*value) : nlohmann::json(nullptr);
            } else if constexpr (is_unique_ptr<T>::value) {
                return value ? SerializeField(*value) : nlohmann::json(nullptr);
            } else {
                return value;
            }
        }

        /**
         * @brief Writes JSON into a field, supporting advanced types.
         *
         * @tparam T Field type.
         * @param field Reference to the field.
         * @param j JSON data.
         *
         * @ingroup SerializationCore
         */
        template<typename T>
        static void DeserializeField(T& field, const nlohmann::json& j) {
            if constexpr (std::is_base_of<SerializableBase, T>::value) {
                field.Deserialize(j);
            } else if constexpr (is_vector<T>::value) {
                field.clear();
                for (const auto& el : j) {
                    typename T::value_type tmp;
                    DeserializeField(tmp, el);
                    field.push_back(std::move(tmp));
                }
            } else if constexpr (is_array<T>::value) {
                size_t idx = 0;
                for (const auto& el : j) {
                    DeserializeField(field[idx++], el);
                }
            } else if constexpr (is_map<T>::value || is_unordered_map<T>::value) {
                field.clear();
                for (auto it = j.begin(); it != j.end(); ++it) {
                    typename T::mapped_type tmp;
                    DeserializeField(tmp, it.value());
                    field[it.key()] = std::move(tmp);
                }
            } else if constexpr (is_optional<T>::value) {
                if (j.is_null()) field.reset();
                else {
                    typename T::value_type tmp;
                    DeserializeField(tmp, j);
                    field = std::move(tmp);
                }
            } else if constexpr (is_unique_ptr<T>::value) {
                if (j.is_null()) field.reset();
                else {
                    field = std::make_unique<typename T::element_type>();
                    DeserializeField(*field, j);
                }
            } else {
                field = j.get<T>();
            }
        }

    private:
        /// Map of field-name → getter producing JSON.
        std::unordered_map<std::string, std::function<nlohmann::json()>> m_fields;

        /// Map of field-name → setter applying JSON to the field.
        std::unordered_map<std::string, std::function<void(const nlohmann::json&)>> m_deserializers;

        // ---------------------------------------------------------------------
        // Internal Type Traits
        // ---------------------------------------------------------------------

        /**
         * @defgroup SerializationHelpers Internal Serialization Traits
         * @ingroup Serialization
         * @brief Internal compile-time helpers for type detection.
         * @{
         */

        /** Detects std::vector */
        template<typename T> struct is_vector : std::false_type {};
        template<typename... Args> struct is_vector<std::vector<Args...>> : std::true_type {};

        /** Detects std::array */
        template<typename T> struct is_array : std::false_type {};
        template<typename U, std::size_t N> struct is_array<std::array<U, N>> : std::true_type {};

        /** Detects std::map */
        template<typename T> struct is_map : std::false_type {};
        template<typename... Args> struct is_map<std::map<Args...>> : std::true_type {};

        /** Detects std::unordered_map */
        template<typename T> struct is_unordered_map : std::false_type {};
        template<typename... Args> struct is_unordered_map<std::unordered_map<Args...>> : std::true_type {};

        /** Detects std::optional */
        template<typename T> struct is_optional : std::false_type {};
        template<typename... Args> struct is_optional<std::optional<Args...>> : std::true_type {};

        /** Detects std::unique_ptr */
        template<typename T> struct is_unique_ptr : std::false_type {};
        template<typename U> struct is_unique_ptr<std::unique_ptr<U>> : std::true_type {};

        /** @} */ // end of SerializationHelpers
    };

} // namespace cp

/** @} */ // end of SerializationCore
/** @} */ // end of Serialization group

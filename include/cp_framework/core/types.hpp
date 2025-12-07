#pragma once

#define GLM_FORCE_INLINE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <cstdint>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/norm.hpp>

#include <string>
#include <string_view>
#include <filesystem>
#include <memory>

namespace cp
{
    using vec2 = glm::vec2;
    using vec3 = glm::vec3;
    using vec4 = glm::vec4;
    using mat4 = glm::mat4;
    using mat3 = glm::mat3;

    using uint = unsigned int;
    using uint8 = uint8_t;
    using uint16 = uint16_t;
    using uint32 = uint32_t;
    using uint64 = uint64_t;
    using int8 = int8_t;
    using int16 = int16_t;
    using int32 = int32_t;
    using int64 = int64_t;

    using bool8 = uint8_t;
    using bool32 = uint32_t;

    using f32 = float;
    using f64 = double;

    using ivec2 = glm::ivec2;
    using ivec3 = glm::ivec3;
    using ivec4 = glm::ivec4;

    using uvec2 = glm::uvec2;
    using uvec3 = glm::uvec3;
    using uvec4 = glm::uvec4;

    using quat = glm::quat;

    using string = std::string;
    using string_view = std::string_view;

    using file_path = std::filesystem::path;

    template <typename TYPE>
    using UPTR = std::unique_ptr<TYPE>;

    template <typename TYPE>
    using SPTR = std::shared_ptr<TYPE>;

    template <typename TYPE>
    using WPTR = std::weak_ptr<TYPE>;

    template <typename TYPE, typename... Args>
    std::unique_ptr<TYPE> M_UPTR(Args &&...args)
    {
        return std::make_unique<TYPE>(std::forward<Args>(args)...);
    }

    template <typename TYPE, typename... Args>
    std::shared_ptr<TYPE> M_SPTR(Args &&...args)
    {
        return std::make_shared<TYPE>(std::forward<Args>(args)...);
    }

    template <typename TYPE>
    std::weak_ptr<TYPE> M_WPTR(const std::shared_ptr<TYPE> &sptr)
    {
        return std::weak_ptr<TYPE>(sptr);
    }
}

#define MAKE_SINGLETON(TypeName)  \
    static TypeName &Get()        \
    {                             \
        static TypeName instance; \
        return instance;          \
    }

#define CP_RULE_OF_FIVE_DELETE(TypeName)            \
    /* Copy Constructor */                          \
    TypeName(const TypeName &) = delete;            \
    /* Copy Assignment  */                          \
    TypeName &operator=(const TypeName &) = delete; \
    /* Move Constructor */                          \
    TypeName(TypeName &&) noexcept = delete;        \
    /* Move Assignment */                           \
    TypeName &operator=(TypeName &&) noexcept = delete;

#define CP_HANDLE_CONVERSION(HANDLE_TYPE, MEMBER)            \
    explicit operator HANDLE_TYPE() const { return MEMBER; } \
    HANDLE_TYPE get() const { return MEMBER; }
#pragma once

#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>

#define CP_VK_DELETE_HANDLE(HANDLE, CALL) \
    do                                    \
    {                                     \
        if ((HANDLE) != VK_NULL_HANDLE)   \
        {                                 \
            CALL;                         \
            (HANDLE) = VK_NULL_HANDLE;    \
        }                                 \
    } while (0)

#define CP_VK_DESTROY(DEVICE, HANDLE, CALL) \
    do                                      \
    {                                       \
        if ((HANDLE) != VK_NULL_HANDLE)     \
        {                                   \
            CALL(DEVICE, HANDLE, nullptr);  \
            (HANDLE) = VK_NULL_HANDLE;      \
        }                                   \
    } while (0)
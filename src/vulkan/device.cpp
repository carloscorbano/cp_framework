#include "cp_framework/vulkan/device.hpp"
#include "cp_framework/vulkan/physicalDevice.hpp"
#include "cp_framework/vulkan/surface.hpp"
#include "cp_framework/vulkan/utils.hpp"
#include "cp_framework/debug/debug.hpp"
#include <set>

namespace cp::vulkan
{
    Device::Device(PhysicalDevice &physDevice, Surface &surface, bool validationLayersEnabled, std::span<const char *> validationLayers, std::span<const char *> deviceExtensions)
    {
        m_familyIndices = utils::findQueueFamilies(physDevice.get(), surface.get());

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties2(physDevice.get(), &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties2> queueFamilies(queueFamilyCount);
        for (auto &qf : queueFamilies)
        {
            qf.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
            qf.pNext = nullptr;
        }

        vkGetPhysicalDeviceQueueFamilyProperties2(physDevice.get(), &queueFamilyCount, queueFamilies.data());

        LOG_INFO("============================================================");
        LOG_INFO("[ QUEUE FAMILIES INFO ]");

        for (uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            const auto &props = queueFamilies[i].queueFamilyProperties;
            std::string bits;
            if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                bits += "GRAPHICS ";
            if (props.queueFlags & VK_QUEUE_COMPUTE_BIT)
                bits += "COMPUTE ";
            if (props.queueFlags & VK_QUEUE_TRANSFER_BIT)
                bits += "TRANSFER ";
            if (props.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
                bits += "SPARSE_BINDING ";
            LOG_INFO("[QueueFamily {}] {} queues | {}", i, props.queueCount, bits);
        }

        LOG_INFO("============================================================");
        LOG_INFO("[ QUEUE FAMILIES IDS ]");
        LOG_INFO("Graphics Queue Family: {}", m_familyIndices.graphicsFamily.value());
        LOG_INFO("Compute Queue Family:  {}", m_familyIndices.computeFamily.value());
        LOG_INFO("Transfer Queue Family: {}", m_familyIndices.transferFamily.value());
        LOG_INFO("Present Queue Family:  {}", m_familyIndices.presentFamily.value());

        LOG_INFO("============================================================");

        // --- 1) Filtrar famílias únicas ---
        std::set<uint32_t> uniqueFamilies = {
            m_familyIndices.graphicsFamily.value(),
            m_familyIndices.presentFamily.value(),
            m_familyIndices.computeFamily.value(),
            m_familyIndices.transferFamily.value()};

        float queuePriority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        for (uint32_t family : uniqueFamilies)
        {
            VkDeviceQueueCreateInfo info{.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
            info.queueFamilyIndex = family;
            info.queueCount = 1;
            info.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(info);
        }

        // --- 2) Query features disponíveis ---
        VkPhysicalDeviceFeatures2 supportedFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        VkPhysicalDeviceVulkan11Features supported11{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
        VkPhysicalDeviceVulkan12Features supported12{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
        VkPhysicalDeviceVulkan13Features supported13{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};

        supportedFeatures.pNext = &supported11;
        supported11.pNext = &supported12;
        supported12.pNext = &supported13;

        vkGetPhysicalDeviceFeatures2(physDevice.get(), &supportedFeatures);

        // --- 3) Escolher as que queremos habilitar ---
        VkPhysicalDeviceFeatures2 enabledFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        VkPhysicalDeviceVulkan11Features enabled11{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
        VkPhysicalDeviceVulkan12Features enabled12{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
        VkPhysicalDeviceVulkan13Features enabled13{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};

        enabledFeatures.pNext = &enabled11;
        enabled11.pNext = &enabled12;
        enabled12.pNext = &enabled13;

        enabled12.timelineSemaphore = VK_TRUE;

        // --- 4) Ativar só o que a GPU suporta ---

        // Vulkan 1.0 features
        if (supportedFeatures.features.samplerAnisotropy)
            enabledFeatures.features.samplerAnisotropy = VK_TRUE;

        if (supportedFeatures.features.sampleRateShading)
            enabledFeatures.features.sampleRateShading = VK_TRUE;

        if (supportedFeatures.features.fillModeNonSolid)
            enabledFeatures.features.fillModeNonSolid = VK_TRUE;

        if (supportedFeatures.features.wideLines)
            enabledFeatures.features.wideLines = VK_TRUE;

        // Vulkan 1.1 / 1.2 / 1.3 extras (só exemplo)
        if (supported12.scalarBlockLayout)
            enabled12.scalarBlockLayout = VK_TRUE;

        if (supported12.descriptorIndexing)
            enabled12.descriptorIndexing = VK_TRUE;

        if (supported13.dynamicRendering)
            enabled13.dynamicRendering = VK_TRUE;

        if (supported13.synchronization2)
            enabled13.synchronization2 = VK_TRUE;

        utils::logDeviceFeatures(
            supportedFeatures, supported11, supported12, supported13,
            enabledFeatures, enabled11, enabled12, enabled13);

        // --- 5) Criar device info ---
        VkDeviceCreateInfo createInfo{.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pNext = &enabledFeatures;

        // Extensões do device
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        // Validation layers
        if (validationLayersEnabled)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        // --- 6) Criar device ---
        if (vkCreateDevice(physDevice.get(), &createInfo, nullptr, &m_device) != VK_SUCCESS)
            LOG_THROW("Failed to create logical device!");

        // --- 7) Obter filas ---
        if (auto func = reinterpret_cast<PFN_vkGetDeviceQueue2>(vkGetDeviceProcAddr(m_device, "vkGetDeviceQueue2")))
        {
            VkDeviceQueueInfo2 queueInfo{.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2};
            queueInfo.queueIndex = 0;

            queueInfo.queueFamilyIndex = m_familyIndices.graphicsFamily.value();
            vkGetDeviceQueue2(m_device, &queueInfo, &m_familyQueues.graphics);

            queueInfo.queueFamilyIndex = m_familyIndices.presentFamily.value();
            vkGetDeviceQueue2(m_device, &queueInfo, &m_familyQueues.present);

            queueInfo.queueFamilyIndex = m_familyIndices.computeFamily.value();
            vkGetDeviceQueue2(m_device, &queueInfo, &m_familyQueues.compute);

            queueInfo.queueFamilyIndex = m_familyIndices.transferFamily.value();
            vkGetDeviceQueue2(m_device, &queueInfo, &m_familyQueues.transfer);
        }
        else
        {
            vkGetDeviceQueue(m_device, m_familyIndices.graphicsFamily.value(), 0, &m_familyQueues.graphics);
            vkGetDeviceQueue(m_device, m_familyIndices.presentFamily.value(), 0, &m_familyQueues.present);
            vkGetDeviceQueue(m_device, m_familyIndices.computeFamily.value(), 0, &m_familyQueues.compute);
            vkGetDeviceQueue(m_device, m_familyIndices.transferFamily.value(), 0, &m_familyQueues.transfer);
        }
    }

    Device::~Device()
    {
        CP_VK_DELETE_HANDLE(m_device, vkDestroyDevice(m_device, nullptr));
    }
} // namespace cp

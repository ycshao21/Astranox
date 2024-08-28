#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanDevice.hpp"
#include "Astranox/platform/vulkan/VulkanSwapchain.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

namespace Astranox
{
    static constexpr std::array<const char*, 1> s_DeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    static void checkDeviceExtensionSupport(Ref<VulkanPhysicalDevice> physicalDevice)
    {
        for (const char* extensionName : s_DeviceExtensions)
        {
            AST_CORE_ASSERT(
                physicalDevice->isExtentionSupported(extensionName),
                "Physical device does not support {0}!", extensionName
            );
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    VulkanDevice::VulkanDevice(const Ref<VulkanPhysicalDevice>& physicalDevice)
        : m_PhysicalDevice(physicalDevice)
    {
        checkDeviceExtensionSupport(m_PhysicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        float queuePriority = 1.0f;
        auto& queueFamilyIndices = m_PhysicalDevice->getQueueIndices();

        VkDeviceQueueCreateInfo graphicsQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(),
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
        queueCreateInfos.push_back(graphicsQueueCreateInfo);

        VkDeviceQueueCreateInfo computeQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queueFamilyIndices.computeFamily.value(),
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
        queueCreateInfos.push_back(computeQueueCreateInfo);

        VkDeviceQueueCreateInfo transferQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queueFamilyIndices.transferFamily.value(),
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
        queueCreateInfos.push_back(transferQueueCreateInfo);

        VkDeviceCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = static_cast<uint32_t>(s_DeviceExtensions.size()),
            .ppEnabledExtensionNames = s_DeviceExtensions.data(),
            .pEnabledFeatures = &m_PhysicalDevice->getFeatures()
        };
        if (VK_ENABLE_VALIDATION_LAYERS)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(VulkanUtils::validationLayers.size());
            createInfo.ppEnabledLayerNames = VulkanUtils::validationLayers.data();
        }

        VK_CHECK(::vkCreateDevice(m_PhysicalDevice->getRaw(), &createInfo, nullptr, &m_Device));

        ::vkGetDeviceQueue(m_Device, queueFamilyIndices.graphicsFamily.value(), 0, &m_GraphicsQueue);
    }

    void VulkanDevice::destroy()
    {
        m_CommandPool = nullptr;

        ::vkDestroyDevice(m_Device, nullptr);
        m_Device = VK_NULL_HANDLE;
    }

    void VulkanDevice::waitIdle() const
    {
        ::vkDeviceWaitIdle(m_Device);
    }
}


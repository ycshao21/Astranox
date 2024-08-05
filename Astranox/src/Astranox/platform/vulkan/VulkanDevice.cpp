#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanDevice.hpp"
#include "Astranox/platform/vulkan/VulkanSwapchain.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

namespace Astranox
{
    VulkanDevice::VulkanDevice(const Ref<VulkanPhysicalDevice>& physicalDevice)
        : m_PhysicalDevice(physicalDevice)
    {
        VulkanUtils::checkDeviceExtensionSupport(m_PhysicalDevice);

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
            .enabledExtensionCount = static_cast<uint32_t>(VulkanUtils::deviceExtensions.size()),
            .ppEnabledExtensionNames = VulkanUtils::deviceExtensions.data(),
            .pEnabledFeatures = &m_PhysicalDevice->getFeatures()
        };
        if (VK_ENABLE_VALIDATION_LAYERS)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(VulkanUtils::validationLayers.size());
            createInfo.ppEnabledLayerNames = VulkanUtils::validationLayers.data();
        }

        VkResult result = ::vkCreateDevice(m_PhysicalDevice->getRaw(), &createInfo, nullptr, &m_Device);
        VK_CHECK(result);

        ::vkGetDeviceQueue(m_Device, queueFamilyIndices.graphicsFamily.value(), 0, &m_GraphicsQueue);
    }

    void VulkanDevice::destroy()
    {
        ::vkDestroyDevice(m_Device, nullptr);
        m_Device = VK_NULL_HANDLE;
    }

    void VulkanDevice::waitIdle() const
    {
        ::vkDeviceWaitIdle(m_Device);
    }
}


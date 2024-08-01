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

        VkDeviceQueueCreateInfo graphicsQueueCreateInfo{};
        graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphicsQueueCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        graphicsQueueCreateInfo.queueCount = 1;
        graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(std::move(graphicsQueueCreateInfo));

        VkDeviceQueueCreateInfo computeQueueCreateInfo{};
        computeQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        computeQueueCreateInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();
        computeQueueCreateInfo.queueCount = 1;
        computeQueueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(std::move(computeQueueCreateInfo));

        VkDeviceQueueCreateInfo transferQueueCreateInfo{};
        transferQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        transferQueueCreateInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();
        transferQueueCreateInfo.queueCount = 1;
        transferQueueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(std::move(transferQueueCreateInfo));

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        if (VK_ENABLE_VALIDATION_LAYERS)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(VulkanUtils::validationLayers.size());
            createInfo.ppEnabledLayerNames = VulkanUtils::validationLayers.data();
        }
        createInfo.enabledExtensionCount = (uint32_t)VulkanUtils::deviceExtensions.size();
        createInfo.ppEnabledExtensionNames = VulkanUtils::deviceExtensions.data();
        createInfo.pEnabledFeatures = &m_PhysicalDevice->getFeatures();

        VkResult result = ::vkCreateDevice(m_PhysicalDevice->getRaw(), &createInfo, nullptr, &m_Device);
        VK_CHECK(result);

        ::vkGetDeviceQueue(m_Device, queueFamilyIndices.graphicsFamily.value(), 0, &m_GraphicsQueue);
    }

    VulkanDevice::~VulkanDevice()
    {
    }

    void VulkanDevice::destroy()
    {
        ::vkDestroyDevice(m_Device, nullptr);
        m_Device = VK_NULL_HANDLE;
    }
}


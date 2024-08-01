#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanPhysicalDevice.hpp"

namespace Astranox
{
    VulkanPhysicalDevice::VulkanPhysicalDevice()
    {
        VkInstance vkInstance = VulkanContext::getInstance();

        uint32_t physicalDeviceCount = 0;
        ::vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);
        AST_CORE_ASSERT(physicalDeviceCount != 0, "Failed to find GPUs with Vulkan support!");
        AST_CORE_DEBUG("Found {0} GPUs with Vulkan support.", physicalDeviceCount);

        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        ::vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevices.data());

        // Find a suitable GPU >>>
        m_PhysicalDevice = VK_NULL_HANDLE;
        for (auto physicalDevice : physicalDevices)
        {
            vkGetPhysicalDeviceProperties(physicalDevice, &m_Properties);
            vkGetPhysicalDeviceFeatures(physicalDevice, &m_Features);
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &m_MemoryProperties);

            if(m_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && m_Features.geometryShader)
            {
                m_PhysicalDevice = physicalDevice;
                break;
            }
        }

        if (m_PhysicalDevice == VK_NULL_HANDLE)
        {
            AST_CORE_ERROR("Failed to find a suitable GPU! Falling back to the first one.");

            m_PhysicalDevice = physicalDevices[0];

            vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Properties);
            vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_Features);
            vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_MemoryProperties);
        }
        AST_CORE_TRACE("Selected GPU: {0}", m_Properties.deviceName);
        // <<< Find a suitable GPU

        // Get queue family properties >>>
        uint32_t queueFamilyCount = 0;
        ::vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
        AST_CORE_ASSERT(queueFamilyCount != 0, "Failed to find queue families!");
        m_QueueFamilyProperties.resize(queueFamilyCount);
        ::vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, m_QueueFamilyProperties.data());
        // <<< Get queue family properties

        // Get supported extensions >>>
        uint32_t extensionCount = 0;
        ::vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, nullptr);
        if (extensionCount != 0)
        {
            std::vector<VkExtensionProperties> extensions(extensionCount);
            ::vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, extensions.data());
            AST_CORE_DEBUG("Found {0} supported extensions for the GPU.", extensionCount);
            for (size_t i = 0; i < extensionCount; i++)
            {
                //AST_CORE_DEBUG("  {0}: {1}", i + 1, extensions[i].extensionName);
                m_SupportedExtensions.emplace(extensions[i].extensionName);
            }
        }
        // <<< Get supported extensions

        // Get supported layers >>>
        uint32_t layerCount = 0;
        ::vkEnumerateDeviceLayerProperties(m_PhysicalDevice, &layerCount, nullptr);
        AST_CORE_ASSERT(layerCount != 0, "Failed to find device layers!");
        m_LayerProperties.resize(layerCount);
        ::vkEnumerateDeviceLayerProperties(m_PhysicalDevice, &layerCount, m_LayerProperties.data());
        // <<< Get supported layers

        findQueueFamilies();
    }

    bool VulkanPhysicalDevice::isExtentionSupported(const std::string& extensionName) const
    {
        return m_SupportedExtensions.find(extensionName) != m_SupportedExtensions.end();
    }

    void VulkanPhysicalDevice::findQueueFamilies()
    {
        // Prioritize compute-only and transfer-only queue families

        for (uint32_t i = 0; i < (uint32_t)m_QueueFamilyProperties.size(); i++)
        {
            const auto& queueFamily = m_QueueFamilyProperties[i];
            if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT && !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                m_QueueFamilyIndices.computeFamily = i;
                break;
            }
        }

        for (uint32_t i = 0; i < (uint32_t)m_QueueFamilyProperties.size(); i++)
        {
            const auto& queueFamily = m_QueueFamilyProperties[i];
            if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT && !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                m_QueueFamilyIndices.transferFamily = i;
                break;
            }
        }

        // Find graphics queue family
        for (uint32_t i = 0; i < (uint32_t)m_QueueFamilyProperties.size(); i++)
        {
            const auto& queueFamily = m_QueueFamilyProperties[i];
            if (!m_QueueFamilyIndices.graphicsFamily.has_value() && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                m_QueueFamilyIndices.graphicsFamily = i;
            }

            if (!m_QueueFamilyIndices.computeFamily.has_value() && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                m_QueueFamilyIndices.computeFamily = i;
            }

            if (!m_QueueFamilyIndices.transferFamily.has_value() && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                m_QueueFamilyIndices.transferFamily = i;
            }

            if (m_QueueFamilyIndices.isComplete())
            {
                return;
            }
        }
        AST_CORE_ASSERT(false, "Failed to find queue families!");
    }

    Ref<VulkanPhysicalDevice> VulkanPhysicalDevice::pick()
    {
        return Ref<VulkanPhysicalDevice>::create();
    }
}


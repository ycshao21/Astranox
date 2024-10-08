#pragma once

#include <string>
#include <set>
#include <optional>

#include <vulkan/vulkan.h>

namespace Astranox
{
    class VulkanPhysicalDevice final: public RefCounted
    {
    public:
        struct QueueFamilyIndices final {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> computeFamily;
            std::optional<uint32_t> transferFamily;

            bool isComplete() {
                return graphicsFamily.has_value()
                    && computeFamily.has_value()
                    && transferFamily.has_value();
            }
        };

    public:
        VulkanPhysicalDevice();
        ~VulkanPhysicalDevice() = default;

        static Ref<VulkanPhysicalDevice> pick();

        VkPhysicalDevice getRaw() { return m_PhysicalDevice; }
        const VkPhysicalDevice getRaw() const { return m_PhysicalDevice; }

        const QueueFamilyIndices& getQueueIndices() const { return m_QueueFamilyIndices; }
        const VkPhysicalDeviceFeatures& getFeatures() const { return m_Features; }
        const std::vector<VkQueueFamilyProperties>& getQueueFamilyProperties() const { return m_QueueFamilyProperties; }
        const VkPhysicalDeviceProperties& getProperties() const { return m_Properties; }
        const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const { return m_MemoryProperties; }
        VkFormat getDepthFormat() const { return m_DepthFormat; }

        bool isExtentionSupported(const std::string& extensionName) const;

    private:
        void findQueueFamilies();

    private:
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;

        VkPhysicalDeviceProperties m_Properties;
        VkPhysicalDeviceFeatures m_Features;
        VkPhysicalDeviceMemoryProperties m_MemoryProperties;

        std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
        QueueFamilyIndices m_QueueFamilyIndices;

        VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;

        std::set<std::string> m_SupportedExtensions;
        std::vector<VkLayerProperties> m_LayerProperties;
    };
}


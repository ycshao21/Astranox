#pragma once
#include "VulkanPhysicalDevice.hpp"
#include "VulkanCommandBuffer.hpp"

namespace Astranox
{
    class VulkanDevice final: public RefCounted
    {
    public:
        VulkanDevice(const Ref<VulkanPhysicalDevice>& physicalDevice);
        ~VulkanDevice() = default;

        void destroy();

    public:
        void waitIdle() const;

    public:
        Ref<VulkanPhysicalDevice> getPhysicalDevice() { return m_PhysicalDevice; }
        const Ref<VulkanPhysicalDevice> getPhysicalDevice() const { return m_PhysicalDevice; }

        VkDevice getRaw() { return m_Device; }
        const VkDevice getRaw() const { return m_Device; }

        VkQueue getGraphicsQueue() { return m_GraphicsQueue; }
        const VkQueue getGraphicsQueue() const { return m_GraphicsQueue; }

    private:
        VkDevice m_Device = VK_NULL_HANDLE;
        Ref<VulkanPhysicalDevice> m_PhysicalDevice = nullptr;

        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    };
}

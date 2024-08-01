#pragma once
#include "VulkanPhysicalDevice.hpp"

namespace Astranox
{
    class VulkanDevice final: public RefCounted
    {
    public:
        VulkanDevice(const Ref<VulkanPhysicalDevice>& physicalDevice);
        ~VulkanDevice();

        void destroy();

        Ref<VulkanPhysicalDevice> getPhysicalDevice() { return m_PhysicalDevice; }
        const Ref<VulkanPhysicalDevice> getPhysicalDevice() const { return m_PhysicalDevice; }

        VkDevice getRaw() { return m_Device; }
        const VkDevice getRaw() const { return m_Device; }

    private:
        VkDevice m_Device = VK_NULL_HANDLE;
        Ref<VulkanPhysicalDevice> m_PhysicalDevice = nullptr;

        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    };
}

#pragma once
#include "Astranox/rendering/GraphicsContext.hpp"

#include <vulkan/vulkan.h>

#include "VulkanPhysicalDevice.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"

namespace Astranox 
{
    class VulkanContext final : public GraphicsContext
    {
    public:
        VulkanContext() = default;
        virtual ~VulkanContext() = default;

        void init(uint32_t& width, uint32_t& height) override;
        void destroy() override;

    public:
        virtual void swapBuffers() override;

    public: // Getters
        static Ref<VulkanContext> get() { return s_Context; }
        static VkInstance getInstance() { return s_Instance; }
        Ref<VulkanPhysicalDevice> getPhysicalDevice() { return m_PhysicalDevice; }
        Ref<VulkanDevice> getDevice() { return m_Device; }
        Ref<VulkanSwapchain> getSwapchain() { return m_Swapchain; }

    private:
        void createInstance();
        void setupDebugMessenger();

    private:
        inline static Ref<VulkanContext> s_Context = nullptr;
        inline static VkInstance s_Instance = VK_NULL_HANDLE;

        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;  // Available only in debug mode

        Ref<VulkanPhysicalDevice> m_PhysicalDevice = nullptr;
        Ref<VulkanDevice> m_Device = nullptr;

        Ref<VulkanSwapchain> m_Swapchain = nullptr;
    };
}

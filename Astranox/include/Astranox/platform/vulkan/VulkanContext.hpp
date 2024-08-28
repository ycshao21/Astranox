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

        /**
         * @brief Explicitly initialize the Vulkan context.
         * @param width Width of the window.
         * @param height Height of the window.
         */
        void init(uint32_t& width, uint32_t& height) override;
        /**
         * @brief Explicitly destroy the Vulkan context.
         */
        void destroy() override;

    public:
        /**
         * @brief Swap the buffers of the swapchain.
         */
        virtual void swapBuffers() override;

    public: // Getters
        static Ref<VulkanContext> get();
        static VkInstance getInstance() { return s_Instance; }
        Ref<VulkanPhysicalDevice> getPhysicalDevice() { return m_PhysicalDevice; }
        Ref<VulkanDevice> getDevice() { return m_Device; }
        Ref<VulkanSwapchain> getSwapchain() { return m_Swapchain; }

    private:
        void createInstance();
        void setupDebugMessenger();

    private:
        inline static VkInstance s_Instance = VK_NULL_HANDLE;

        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;  // Available only in debug mode

        Ref<VulkanPhysicalDevice> m_PhysicalDevice = nullptr;
        Ref<VulkanDevice> m_Device = nullptr;

        Ref<VulkanSwapchain> m_Swapchain = nullptr;
    };
}

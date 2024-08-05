#pragma once
#include "Astranox/core/RefCounted.hpp"

#include "VulkanDevice.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanShader.hpp"
#include "VulkanPipeline.hpp"

#include <vector>

namespace Astranox
{
    class VulkanSwapchain: public RefCounted
    {
    public:
        VulkanSwapchain(Ref<VulkanDevice> device);
        virtual ~VulkanSwapchain() = default;

        void createSurface();
        void createSwapchain();

        void destroy();

    public:
        void drawFrame();

        void beginFrame() const;
        void present() const;

    public:
        uint32_t getWidth() const { return m_SwapchainExtent.width; }
        uint32_t getHeight() const { return m_SwapchainExtent.height; }

        VkRenderPass getVkRenderPass() { return m_RenderPass; }

    private:
        void chooseSurfaceFormat();

        void getQueueIndices();
        void getSwapchainImages();

        void acquireNextImage();

        VkFramebuffer getCurrentFramebuffer() { return m_Framebuffers[m_CurrentImageIndex]; }
        VkCommandBuffer getCurrentCommandBuffer() { return m_CommandBuffers[m_CurrentFramebufferIndex]; }

        void createRenderPass();
        void createFramebuffers();
        void createCommandBuffers();
        void createSyncObjects();

    private:
        Ref<VulkanDevice> m_Device = nullptr;

        uint32_t m_GraphicsQueueIndex = 0;
        uint32_t m_PresentQueueIndex = 0;

        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        VkFormat m_ImageFormat{};
        VkColorSpaceKHR m_ColorSpace{};

        VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
        VkExtent2D m_SwapchainExtent{};

        struct SwapchainImage
        {
            VkImage image;
            VkImageView imageView;
        };
        std::vector<SwapchainImage> m_Images;

        VkRenderPass m_RenderPass = VK_NULL_HANDLE;

        std::vector<VkFramebuffer> m_Framebuffers;
        uint32_t m_CurrentFramebufferIndex = 0;
        uint32_t m_CurrentImageIndex = 0;

        Ref<VulkanCommandPool> m_CommandPool = nullptr;
        uint32_t m_MaxFramesInFlight = 2;
        std::vector<VkCommandBuffer> m_CommandBuffers;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;

        // Remove
		Ref<VulkanShader> m_Shader = nullptr;
		Ref<VulkanPipeline> m_Pipeline = nullptr;
    };
}

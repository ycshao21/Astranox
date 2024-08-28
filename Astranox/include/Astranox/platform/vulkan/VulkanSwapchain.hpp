#pragma once
#include "Astranox/core/RefCounted.hpp"

#include "VulkanDevice.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanShader.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanDescriptorManager.hpp"

#include "Astranox/rendering/PerspectiveCamera.hpp"
#include "Astranox/rendering/VertexBuffer.hpp"
#include "Astranox/rendering/IndexBuffer.hpp"
#include "Astranox/rendering/UniformBuffer.hpp"
#include "Astranox/rendering/Texture2D.hpp"

#include <vector>
#include "vk_mem_alloc.h"

namespace Astranox
{
    class VulkanSwapchain: public RefCounted
    {
    public:
        VulkanSwapchain(Ref<VulkanDevice> device);
        virtual ~VulkanSwapchain() = default;

        void createSurface();
        void createSwapchain(uint32_t width, uint32_t height);

        void destroy();

    public:
        void resize(uint32_t width, uint32_t height);

        void beginFrame();
        void present();

    public:
        uint32_t getWidth() const { return m_SwapchainExtent.width; }
        uint32_t getHeight() const { return m_SwapchainExtent.height; }
        VkExtent2D getExtent() const { return m_SwapchainExtent; }

        VkRenderPass getRenderPass() { return m_RenderPass; }
        uint32_t getImageCount() const { return m_Images.size(); }

        VkFramebuffer getCurrentFramebuffer() { return m_Framebuffers[m_CurrentImageIndex]; }
        VkCommandBuffer getCurrentCommandBuffer();

    private:
        void chooseSurfaceFormat();

        void getQueueIndices();
        void getSwapchainImages();

        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();

        VkImageView createImageView(
            VkImage image,
            VkFormat format,
            VkImageAspectFlags aspectFlags,
            uint32_t mipLevels
        );

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
        //uint32_t m_CurrentFrameIndex = 0;
        uint32_t m_CurrentImageIndex = 0;

        std::vector<VkCommandBuffer> m_CommandBuffers;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;

        //struct ColorAttachment
        //{
        //    VkImage image = VK_NULL_HANDLE;
        //    VkDeviceMemory memory = VK_NULL_HANDLE;
        //    VkImageView imageView = VK_NULL_HANDLE;
        //} m_ColorAttachment;

        struct DepthStencil
        {
            VkImage image = VK_NULL_HANDLE;
            VmaAllocation allocation;
            VkImageView imageView = VK_NULL_HANDLE;
        } m_DepthStencil;
    };
}

#pragma once
#include <vector>
#include "Astranox/core/RefCounted.hpp"
#include "VulkanDevice.hpp"

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

    private:
        void chooseSurfaceFormat();

        void getQueueIndices();
        void getSwapchainImages();

        // [TODO] Remove

        //void createPipeline();

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
    };
}

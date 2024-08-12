#include "pch.hpp"
#include "Astranox/rendering/Renderer.hpp"

namespace Astranox
{
    static RendererConfig s_RendererConfig;

    void Renderer::init()
    {
        s_RendererConfig.framesInFlight = VulkanContext::get()->getSwapchain()->getImageCount();

        // Load shaders

        // Load textures

        // Initialize renderer api
        s_RendererAPI = Ref<RendererAPI>::create();
    }

    void Renderer::shutdown()
    {
        s_RendererAPI = nullptr;
    }

    const RendererConfig& Renderer::getConfig()
    {
        return s_RendererConfig;
    }

    //VkSampleCountFlagBits Renderer::getMaxUsableSampleCount()
    //{
    //    auto physicalDevice = VulkanContext::get()->getDevice()->getPhysicalDevice();
    //    VkPhysicalDeviceProperties properties = physicalDevice->getProperties();

    //    VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts
    //                              & properties.limits.framebufferDepthSampleCounts;

    //    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    //    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    //    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    //    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    //    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    //    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    //    return VK_SAMPLE_COUNT_1_BIT;
    //}

}


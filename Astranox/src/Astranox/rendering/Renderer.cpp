#include "pch.hpp"
#include "Astranox/rendering/Renderer.hpp"
#include "Astranox/platform/vulkan/VulkanRenderer.hpp"

#include "Astranox/core/Application.hpp"


namespace Astranox
{
    static RendererConfig s_RendererConfig;

    static RendererAPI* initRendererAPI()
    {
        switch (RendererAPI::getType())
        {
            case RendererAPI::Type::None: { AST_CORE_ASSERT(false, "RendererAPI::Type::None is not supported!"); break; }
            case RendererAPI::Type::Vulkan: { return new VulkanRenderer(); }
        }

        AST_CORE_ASSERT(false, "Unknown RendererAPI::Type!");
        return nullptr;
    }

    void Renderer::init()
    {
        s_RendererConfig.framesInFlight = VulkanContext::get()->getSwapchain()->getImageCount();

        // Load shaders

        // Load textures

        // Initialize renderer api
        s_RendererAPI = initRendererAPI();
    }

    void Renderer::shutdown()
    {
        delete s_RendererAPI;
    }

    uint32_t Renderer::getCurrentFrameIndex()
    {
        return Application::get().getCurrentFrameIndex();
    }

    const RendererConfig& Renderer::getConfig()
    {
        return s_RendererConfig;
    }

    void Renderer::beginFrame()
    {
        s_RendererAPI->beginFrame();
    }

    void Renderer::endFrame()
    {
        s_RendererAPI->endFrame();
    }

    void Renderer::beginRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, Ref<VulkanPipeline> pipeline, const std::vector<VkDescriptorSet>& descriptorSets)
    {
        s_RendererAPI->beginRenderPass(commandBuffer, renderPass, pipeline, descriptorSets);
    }

    void Renderer::endRenderPass(VkCommandBuffer commandBuffer)
    {
        s_RendererAPI->endRenderPass(commandBuffer);
    }

    void Renderer::renderMesh(VkCommandBuffer commandBuffer, Ref<VulkanPipeline> pipeline, Mesh& mesh, uint32_t instanceCount)
    {
        s_RendererAPI->renderMesh(commandBuffer, pipeline, mesh, instanceCount);
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


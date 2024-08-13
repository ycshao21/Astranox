#pragma once
#include "RendererAPI.hpp"

#include "Astranox/platform/vulkan/VulkanContext.hpp"

namespace Astranox
{
    struct RendererConfig
    {
        uint32_t framesInFlight;
    };

    class Renderer
    {
    public:
		static void init();
        static void shutdown();

    public:
        static uint32_t getCurrentFrameIndex();

        static const RendererConfig& getConfig();

    public:
        static void beginFrame();
        static void endFrame();

        static void beginRenderPass(
            VkCommandBuffer commandBuffer,
            VkRenderPass renderPass,
            Ref<VulkanPipeline> pipeline,
            const std::vector<VkDescriptorSet>& descriptorSets);

        static void endRenderPass(VkCommandBuffer commandBuffer);

        static void renderMesh(
            VkCommandBuffer commandBuffer,
            Ref<VulkanPipeline> pipeline,
            Mesh& mesh,
            uint32_t instanceCount);

    private:
        //static VkSampleCountFlagBits getMaxUsableSampleCount();

    private:
        inline static RendererAPI* s_RendererAPI = nullptr;
    };
}

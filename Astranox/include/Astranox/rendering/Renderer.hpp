#pragma once
#include "RendererAPI.hpp"

#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/rendering/Texture2D.hpp"

namespace Astranox
{
    // TEMP
    struct CameraData
    {
        alignas(16) glm::mat4 viewProjection;
    };

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

        static void renderGeometry(
            VkCommandBuffer commandBuffer,
            Ref<VulkanPipeline> pipeline,
            Ref<VulkanDescriptorManager> dm,
            Ref<VertexBuffer> vertexBuffer,
            Ref<IndexBuffer> indexBuffer,
            uint32_t indexCount);

        static void renderMesh(
            VkCommandBuffer commandBuffer,
            Ref<VulkanPipeline> pipeline,
            Mesh& mesh,
            uint32_t instanceCount);

    public:
        static Ref<Texture2D> getWhiteTexture();

    private:
        //static VkSampleCountFlagBits getMaxUsableSampleCount();

    private:
        inline static RendererAPI* s_RendererAPI = nullptr;

        inline static Ref<Texture2D> s_WhiteTexture = nullptr;
    };
}

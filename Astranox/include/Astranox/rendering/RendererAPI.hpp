#pragma once
#include "Mesh.hpp"
#include <vulkan/vulkan.h>
#include "Astranox/platform/vulkan/VulkanPipeline.hpp"

namespace Astranox
{
    class RendererAPI
    {
    public:
        enum class Type
        {
            None = 0,
            Vulkan = 1
        };
    public:
		virtual void beginFrame() = 0;
		virtual void endFrame() = 0;

		virtual void beginRenderPass(
            VkCommandBuffer commandBuffer,
            VkRenderPass renderPass,
            Ref<VulkanPipeline> pipeline,
            const std::vector<VkDescriptorSet>& descriptorSets) = 0;
		virtual void endRenderPass(VkCommandBuffer commandBuffer) = 0;

		virtual void renderMesh(
			VkCommandBuffer commandBuffer,
			Ref<VulkanPipeline> pipeline,
			Mesh& mesh,
			uint32_t instanceCount) = 0;

    public:
        static Type getType() { return s_Type; }

    private:
        inline static Type s_Type = Type::Vulkan;
    };
}

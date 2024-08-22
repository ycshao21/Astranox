#pragma once

#include "VulkanPipeline.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanPipeline.hpp"
#include "Astranox/rendering/RendererAPI.hpp"

namespace Astranox
{
	class VulkanRenderer: public RendererAPI
	{
	public:
		VulkanRenderer();
		virtual ~VulkanRenderer() = default;

		void beginFrame() override;
		void endFrame() override;

		void beginRenderPass(
			VkCommandBuffer commandBuffer,
			VkRenderPass renderPass,
			Ref<VulkanPipeline> pipeline,
			const std::vector<VkDescriptorSet>& descriptorSets) override;

		void endRenderPass(VkCommandBuffer commandBuffer) override;

		void renderMesh(
			VkCommandBuffer commandBuffer,
			Ref<VulkanPipeline> pipeline,
			Mesh& mesh,
			uint32_t instanceCount) override;

        void renderGeometry(
            VkCommandBuffer commandBuffer,
            Ref<VulkanPipeline> pipeline,
            Ref<VertexBuffer> vertexBuffer,
            Ref<IndexBuffer> indexBuffer,
            uint32_t indexCount = 1) override;
	};
}

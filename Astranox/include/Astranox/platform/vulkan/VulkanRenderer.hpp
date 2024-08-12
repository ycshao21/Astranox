#pragma once

#include "VulkanPipeline.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanPipeline.hpp"
#include "Astranox/rendering/VertexBuffer.hpp"
#include "Astranox/rendering/IndexBuffer.hpp"

namespace Astranox
{
	class VulkanRenderer: public RefCounted
	{
	public:
		VulkanRenderer();
		virtual ~VulkanRenderer() = default;

		void init();
		void shutdown();

		void beginFrame();
		void endFrame();

		void beginRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, Ref<VulkanPipeline> pipeline, const std::vector<VkDescriptorSet>& descriptorSets);
		void endRenderPass(VkCommandBuffer commandBuffer);

		void render(
			VkCommandBuffer commandBuffer,
			Ref<VulkanPipeline> pipeline,
			Ref<VertexBuffer> vertexBuffer,
			Ref<IndexBuffer> indexBuffer,
			uint32_t instanceCount
		);
	};
}

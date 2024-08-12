#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanRenderer.hpp"

#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanVertexBuffer.hpp"
#include "Astranox/platform/vulkan/VulkanIndexBuffer.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

namespace Astranox
{
    VulkanRenderer::VulkanRenderer()
    {
    }

    void VulkanRenderer::init()
    {

    }

    void VulkanRenderer::shutdown()
    {
    }

    void VulkanRenderer::beginFrame()
    {
        auto swapchain = VulkanContext::get()->getSwapchain();

        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
            .pInheritanceInfo = nullptr
        };
        VK_CHECK(::vkBeginCommandBuffer(swapchain->getCurrentCommandBuffer(), &beginInfo));
    }

    void VulkanRenderer::endFrame()
    {
        auto swapchain = VulkanContext::get()->getSwapchain();
        VK_CHECK(::vkEndCommandBuffer(swapchain->getCurrentCommandBuffer()));
    }

    void VulkanRenderer::beginRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, Ref<VulkanPipeline> pipeline, const std::vector<VkDescriptorSet>& descriptorSets)
    {
        auto swapchain = VulkanContext::get()->getSwapchain();

        std::array<VkClearValue, 2> clearValues;
        clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0u };

        VkRenderPassBeginInfo renderPassInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = renderPass,
            .framebuffer = swapchain->getCurrentFramebuffer(),
            .renderArea = {
                .offset = { 0, 0 },
                .extent = swapchain->getExtent()
            },
            .clearValueCount = static_cast<uint32_t>(clearValues.size()),
            .pClearValues = clearValues.data()
        };
        ::vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(swapchain->getExtent().width),
            .height = static_cast<float>(swapchain->getExtent().height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {
            .offset = { 0, 0 },
            .extent = swapchain->getExtent()
        };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 0, 1, &descriptorSets[swapchain->getCurrentFrameIndex()], 0, nullptr);

        // Bind pipeline
        VkPipeline graphicsPipeline = pipeline->getRaw();
        ::vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    }

    void VulkanRenderer::endRenderPass(VkCommandBuffer commandBuffer)
    {
        ::vkCmdEndRenderPass(commandBuffer);
    }

    void VulkanRenderer::render(
        VkCommandBuffer commandBuffer,
        Ref<VulkanPipeline> pipeline,
        Ref<VertexBuffer> vertexBuffer,
        Ref<IndexBuffer> indexBuffer,
        uint32_t instanceCount
    )
    {
        //vertexBuffer->bind();
        // ^^^^
        VkDeviceSize offsets[] = { 0 };
        VkBuffer vb = vertexBuffer.as<VulkanVertexBuffer>()->getRaw();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vb, offsets);

        //indexBuffer->bind();
        // ^^^^
        VkBuffer ib = indexBuffer.as<VulkanIndexBuffer>()->getRaw();
        vkCmdBindIndexBuffer(commandBuffer, ib, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(commandBuffer, indexBuffer->getCount(), instanceCount, 0, 0, 0);
    }
}


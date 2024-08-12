#pragma once

#include <vulkan/vulkan.h>
#include "Astranox/core/RefCounted.hpp"

namespace Astranox
{
    class VulkanCommandPool: public RefCounted
    {
    public:
        VulkanCommandPool();
        virtual ~VulkanCommandPool();

        VkCommandBuffer allocateCommandBuffer();
        std::vector<VkCommandBuffer> allocateCommandBuffers(uint32_t count);

        VkCommandPool getGraphicsCommandPool() { return m_GraphicsCommandPool; }
        const VkCommandPool getGraphicsCommandPool() const { return m_GraphicsCommandPool; }

        void beginOneTimeBuffer(VkCommandBuffer commandBuffer);
        void endOneTimeBuffer(VkCommandBuffer commandBuffer);

    private:
        VkCommandPool m_GraphicsCommandPool = VK_NULL_HANDLE;
    };
}

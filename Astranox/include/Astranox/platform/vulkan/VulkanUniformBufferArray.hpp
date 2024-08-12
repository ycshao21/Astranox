#pragma once
#include "Astranox/rendering/UniformBufferArray.hpp"
#include "VulkanContext.hpp"

namespace Astranox
{
    class VulkanUniformBufferArray : public UniformBufferArray
    {
    public:
        VulkanUniformBufferArray(uint32_t bytes);
        virtual ~VulkanUniformBufferArray() = default;

        Ref<UniformBuffer> getBuffer(uint32_t frameIndex) override
        {
            return m_UniformBuffers[frameIndex];
        }

        Ref<UniformBuffer> getCurrentBuffer() override
        {
            auto swapchain = VulkanContext::get()->getSwapchain();
            uint32_t currentFrameIndex = swapchain->getCurrentFrameIndex();
            return m_UniformBuffers[currentFrameIndex];
        }

    private:
        std::vector<Ref<UniformBuffer>> m_UniformBuffers;
    };
}

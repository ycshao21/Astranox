#pragma once
#include "Astranox/rendering/UniformBufferArray.hpp"
#include "Astranox/rendering/Renderer.hpp"
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
            uint32_t currentFrameIndex = Renderer::getCurrentFrameIndex();
            return m_UniformBuffers[currentFrameIndex];
        }

    private:
        std::vector<Ref<UniformBuffer>> m_UniformBuffers;
    };
}

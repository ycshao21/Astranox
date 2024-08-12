#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanUniformBufferArray.hpp"

#include "Astranox/rendering/Renderer.hpp"

namespace Astranox
{
    VulkanUniformBufferArray::VulkanUniformBufferArray(uint32_t bytes)
    {
        uint32_t framesInFlight = Renderer::getConfig().framesInFlight;
        m_UniformBuffers.resize(framesInFlight);

        for (uint32_t i = 0; i < framesInFlight; i++)
        {
            m_UniformBuffers[i] = UniformBuffer::create(bytes);
        }
    }
}
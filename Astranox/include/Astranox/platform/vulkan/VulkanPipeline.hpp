#pragma once
#include "Astranox/core/RefCounted.hpp"
#include "VulkanShader.hpp"
#include <vulkan/vulkan.h>

namespace Astranox
{
    class VulkanPipeline: public RefCounted
    {
    public:
        VulkanPipeline(Ref<Shader> shader);
        virtual ~VulkanPipeline();

        void createPipeline();

    public:
        VkPipeline getRaw() { return m_Pipeline; }

    private:
        Ref<VulkanShader> m_Shader = nullptr;

        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_Pipeline = VK_NULL_HANDLE;
        VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
    };
}

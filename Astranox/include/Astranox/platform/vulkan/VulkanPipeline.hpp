#pragma once
#include "Astranox/core/RefCounted.hpp"

#include "Astranox/rendering/VertexBufferLayout.hpp"
#include "VulkanShader.hpp"

namespace Astranox
{
    class VulkanPipeline: public RefCounted
    {
    public:
        VulkanPipeline(Ref<Shader> shader, const VertexBufferLayout& vertexBufferLayout);
        virtual ~VulkanPipeline();

        void createPipeline();

    public:
        VkPipeline getRaw() { return m_Pipeline; }
        VkPipelineLayout getLayout() { return m_PipelineLayout; }

    private:
        Ref<VulkanShader> m_Shader = nullptr;
        VertexBufferLayout m_VertexBufferLayout;

        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_Pipeline = VK_NULL_HANDLE;
        VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
    };
}

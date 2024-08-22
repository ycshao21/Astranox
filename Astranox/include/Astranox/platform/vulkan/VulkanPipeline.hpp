#pragma once
#include "Astranox/core/RefCounted.hpp"

#include "Astranox/rendering/VertexBufferLayout.hpp"
#include "VulkanShader.hpp"

namespace Astranox
{
    struct PipelineSpecification
    {
        Ref<Shader> shader;
        VertexBufferLayout vertexBufferLayout;
        bool depthTestEnable = true;
        bool depthWriteEnable = false;
    };

    class VulkanPipeline: public RefCounted
    {
    public:
        VulkanPipeline(const PipelineSpecification& specification);
        virtual ~VulkanPipeline();

    public:
        VkPipeline getRaw() { return m_Pipeline; }
        VkPipelineLayout getLayout() { return m_PipelineLayout; }

    private:
        void init();

    private:
        PipelineSpecification m_Specification;

        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_Pipeline = VK_NULL_HANDLE;
        VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
    };
}

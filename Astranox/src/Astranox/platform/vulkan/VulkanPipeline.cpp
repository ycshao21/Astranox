#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanPipeline.hpp"

#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

#include "Astranox/rendering/Renderer.hpp"
#include "Astranox/rendering/Mesh.hpp"
#include "Astranox/rendering/VertexBufferLayout.hpp"

namespace Astranox
{
    VulkanPipeline::VulkanPipeline(const PipelineSpecification& specification)
        : m_Specification(specification)
    {
        init();
    }

    VulkanPipeline::~VulkanPipeline()
    {
        auto device = VulkanContext::get()->getDevice();

        ::vkDestroyPipeline(device->getRaw(), m_Pipeline, nullptr);
        ::vkDestroyPipelineLayout(device->getRaw(), m_PipelineLayout, nullptr);
        ::vkDestroyPipelineCache(device->getRaw(), m_PipelineCache, nullptr);
    }

    void VulkanPipeline::init()
    {
        auto device = VulkanContext::get()->getDevice();

        Ref<VulkanShader> shader = m_Specification.shader.as<VulkanShader>();
        const auto& descriptorSetLayouts = shader->getDescriptorSetLayouts();
        const auto& pushConstantRanges = shader->getPushConstantRanges();

        // Pipeline Layout >>>
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
            .pSetLayouts = descriptorSetLayouts.data(),
            .pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
            .pPushConstantRanges = pushConstantRanges.data(),
        };
        VK_CHECK(::vkCreatePipelineLayout(device->getRaw(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout));
        // <<< Pipeline Layout


        // Pipeline >>>
        // (1) Vertex Input
        auto& vbLayout = m_Specification.vertexBufferLayout;
        std::vector<VkVertexInputBindingDescription> vertexInputBindings{
            {
                .binding = 0,
                .stride = vbLayout.getStride(),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            }
        };

        std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;
        auto& elements = vbLayout.getElements();
        for (uint32_t i = 0; i < static_cast<uint32_t>(elements.size()); i++)
        {
            const auto& e = elements[i];
            VkVertexInputAttributeDescription attribute = {
                .location = i,
                .binding = 0,
                .format = VulkanUtils::shaderDataTypeToVkFormat(e.dataType),
                .offset = e.offset
            };
            vertexInputAttributes.push_back(attribute);
        }

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size()),
            .pVertexBindingDescriptions = vertexInputBindings.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size()),
            .pVertexAttributeDescriptions = vertexInputAttributes.data(),
        };

        // (2) Input Assembly
        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkBool32 primitiveRestartEnable = VK_FALSE;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = topology,
            .primitiveRestartEnable = primitiveRestartEnable,
        };

        // (3) Tessellation
        // We don't use it for now

        // (4) Viewport and Scissor
        auto swapchain = VulkanContext::get()->getSwapchain();

        VkPipelineViewportStateCreateInfo viewportStateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        };

        // (5) Rasterization
        VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
        VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
        float lineWidth = 1.0f;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = polygonMode,
            .cullMode = cullMode,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = lineWidth
        };

        // (6) Multisample
        VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
        VkPipelineMultisampleStateCreateInfo multisampleInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = msaaSamples,
            .sampleShadingEnable = VK_TRUE,
            .minSampleShading = 0.2f,
        };

        // (7) Depth and Stencil
        VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = m_Specification.depthTestEnable ? VK_TRUE : VK_FALSE,
            .depthWriteEnable = m_Specification.depthWriteEnable ? VK_TRUE : VK_FALSE,
            .depthCompareOp = depthCompareOp,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = 0.0f,
            .maxDepthBounds = 1.0f,
        };

        // (8) Color Blending
        VkPipelineColorBlendAttachmentState colorBlendAttachment = {
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };

        VkPipelineColorBlendStateCreateInfo colorBlendingInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment,
            .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f },
        };

        // (9) Dynamic
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data(),
        };

        auto& shaderStages = shader->getShaderStageCreateInfos();

        VkGraphicsPipelineCreateInfo pipelineInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssemblyInfo,
            .pTessellationState = nullptr,
            .pViewportState = &viewportStateInfo,
            .pRasterizationState = &rasterizationInfo,
            .pMultisampleState = &multisampleInfo,
            .pDepthStencilState = &depthStencilInfo,
            .pColorBlendState = &colorBlendingInfo,
            .pDynamicState = &dynamicStateInfo,
            .layout = m_PipelineLayout,
            .renderPass = swapchain->getRenderPass(),
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1,
        };

        VK_CHECK(::vkCreateGraphicsPipelines(device->getRaw(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline));
        // <<< Pipeline


        // Pipeline Cache >>>
        VkPipelineCacheCreateInfo pipelineCacheInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .initialDataSize = 0,
            .pInitialData = nullptr,
        };

        VK_CHECK(::vkCreatePipelineCache(device->getRaw(), &pipelineCacheInfo, nullptr, &m_PipelineCache));
        // <<< Pipeline Cache
    }
}

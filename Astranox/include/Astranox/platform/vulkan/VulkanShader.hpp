#pragma once

#include "Astranox/rendering/Shader.hpp"
#include "VulkanShaderCompiler.hpp"

#include <vulkan/vulkan.h>

namespace Astranox
{
    struct UniformBufferInfo
    {
        uint32_t count;
        VkShaderStageFlags shaderStage;
        std::string name;
    };

    struct ImageSamplerInfo
    {
        uint32_t arraySize;
        VkShaderStageFlags shaderStage;
        std::string name;
    };

    struct ShaderDescriptorSetInfo
    {
        std::map<uint32_t, UniformBufferInfo> uniformBufferInfos;  // [binding, info]
        std::map<uint32_t, ImageSamplerInfo> imageSamplerInfos;    // [binding, info]

        std::map<std::string, VkWriteDescriptorSet> writeDescriptorSets;  // [name, wd]
    };

    class VulkanShader : public Shader
    {
    public:
        VulkanShader() = default;
        virtual ~VulkanShader();

        void createDescriptorSetLayouts();

    public:
        void bind() override;
        void unbind() override;

    public:
        const std::string& getName() const override { return m_Name; }

        VkDescriptorSetLayout getDescriptorSetLayout(uint32_t setIndex) { return m_DescriptorSetLayouts[setIndex]; }
        const std::vector<VkDescriptorSetLayout>& getDescriptorSetLayouts() { return m_DescriptorSetLayouts; }

        const std::vector<VkPushConstantRange>& getPushConstantRanges() { return m_PushConstantRanges; }

        std::vector<VkPipelineShaderStageCreateInfo>& getShaderStageCreateInfos() { return m_ShaderStages; }

        // TEMP
        void setDescriptorSetInfo(const ShaderDescriptorSetInfo& info) { m_ShaderDescriptorSetInfo = info; }
        ShaderDescriptorSetInfo& getDescriptorSetInfo() { return m_ShaderDescriptorSetInfo; }

    private:
        void destroy();

        void createShaders(const std::map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData);

    private:
        std::string m_Name;
        std::filesystem::path m_Filepath;

        ShaderDescriptorSetInfo m_ShaderDescriptorSetInfo;

        std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;

        std::vector<VkPushConstantRange> m_PushConstantRanges;  // Unset

        std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;

    private:
        friend class VulkanShaderCompiler;
        friend class VulkanPipeline;
    };
}

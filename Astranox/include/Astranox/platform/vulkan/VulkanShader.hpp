#pragma once

#include "Astranox/rendering/Shader.hpp"

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
        friend class VulkanPipeline;

    public:
        VulkanShader(const std::string& name, const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
        virtual ~VulkanShader();

        void createDescriptorSetLayouts();
        void destroy();

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
        std::vector<char> readCompiledShaderFile(const std::filesystem::path& codeFile);
        void createShaders(const std::map<VkShaderStageFlagBits, std::vector<char>>& shaderData);

    private:
        std::string m_Name;
        ShaderDescriptorSetInfo m_ShaderDescriptorSetInfo;

        std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;

        std::vector<VkPushConstantRange> m_PushConstantRanges;  // Unset

        std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
    };
}

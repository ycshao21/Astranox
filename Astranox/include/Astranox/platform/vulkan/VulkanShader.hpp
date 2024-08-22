#pragma once

#include "Astranox/rendering/Shader.hpp"

#include <vulkan/vulkan.h>

namespace Astranox
{
    struct UniformBufferInfo
    {
        VkShaderStageFlags shaderStage; 
        std::string debugName;
    };

    class VulkanShader : public Shader
    {
        friend class VulkanPipeline;

    public:
        VulkanShader(const std::string& name, const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
        virtual ~VulkanShader();

        void setUniformBufferInfos(const std::map<uint32_t, UniformBufferInfo>& uniformBufferInfos)
        {
            m_UniformBufferInfos = uniformBufferInfos;
        }

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

    private:
        std::vector<char> readCompiledShaderFile(const std::filesystem::path& codeFile);
        void createShaders(const std::map<VkShaderStageFlagBits, std::vector<char>>& shaderData);

    private:
        std::string m_Name;

        std::map<uint32_t, UniformBufferInfo> m_UniformBufferInfos;

        std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;

        std::vector<VkPushConstantRange> m_PushConstantRanges;  // Unset

        std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
    };
}

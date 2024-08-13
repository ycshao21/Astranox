#pragma once

#include "Astranox/rendering/Shader.hpp"

#include <vulkan/vulkan.h>

namespace Astranox
{
    class VulkanShader : public Shader
    {
        friend class VulkanPipeline;

    public:
        VulkanShader(const std::string& name, const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
        virtual ~VulkanShader();

        virtual void createShaders(const std::vector<char>& vertexCode, const std::vector<char>& fragmentCode) override;
        void createDescriptorSetLayout();
        void destroy();


        virtual void bind() override;
        virtual void unbind() override;

        virtual const std::string& getName() const override { return m_Name; }

        VkShaderModule getVertexShader() const { return m_VertexShaderModule; }
        VkShaderModule getFragmentShader() const { return m_FragmentShaderModule; }

        std::vector<VkDescriptorSetLayout>& getDescriptorSetLayouts() { return m_DescriptorSetLayouts; }
        std::vector<VkPushConstantRange>& getPushConstantRanges() { return m_PushConstantRanges; }

        std::vector<VkPipelineShaderStageCreateInfo>& getShaderStageCreateInfos() { return m_ShaderStages; }

    private:
        std::vector<char> readCompiledShaderFile(const std::filesystem::path& codeFile);

    private:
        std::string m_Name;

        VkShaderModule m_VertexShaderModule = VK_NULL_HANDLE;
        VkShaderModule m_FragmentShaderModule = VK_NULL_HANDLE;

        std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;  // Unset
        std::vector<VkPushConstantRange> m_PushConstantRanges;  // Unset

        std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
    };
}

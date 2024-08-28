#pragma once
#include <filesystem>
#include <vulkan/vulkan.h>

namespace Astranox
{
    class VulkanShader;

    class VulkanShaderCompiler: public RefCounted
    {
    public:
        VulkanShaderCompiler(const std::filesystem::path& shaderFilepath);
        virtual ~VulkanShaderCompiler() = default;

        static Ref<VulkanShader> compile(const std::filesystem::path& shaderFilepath);

        const std::map<VkShaderStageFlagBits, std::vector<uint32_t>>& getShaderData() { return m_ShaderData; }

    private:
        void process();

        std::string readFile(const std::filesystem::path& filepath);
        std::map<VkShaderStageFlagBits, std::string> parseShader(const std::string& srcCode);
        void compileOrGetVulkanBinaries(const std::map<VkShaderStageFlagBits, std::string>& shaderSources);

        void reflect(VkShaderStageFlagBits stage, const std::vector<uint32_t>& spirv);

    private:
        std::filesystem::path m_ShaderFilepath;

        std::map<VkShaderStageFlagBits, std::vector<uint32_t>> m_ShaderData;
    };
}

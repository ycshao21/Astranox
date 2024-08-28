#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanShaderCompiler.hpp"
#include "Astranox/platform/vulkan/VulkanShader.hpp"
#include "Astranox/core/Timer.hpp"

#include "shaderc/glslc/src/file_compiler.h"
#include "spirv_cross/spirv_cross.hpp"


namespace Astranox
{
    namespace Utils
    {
        static VkShaderStageFlagBits shaderTypeFromString(const std::string& type)
        {
            if (type == "vertex")
            {
                return VK_SHADER_STAGE_VERTEX_BIT;
            }
            if (type == "fragment")
            {
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            }

            AST_CORE_ASSERT(false, "Unknown shader type");
            return static_cast<VkShaderStageFlagBits>(0);
        }

        static std::filesystem::path getCacheDirectory()
        {
            return std::filesystem::path("assets/cache/shader/Vulkan");
        }

        static void createCacheDirectoryIfNeeded()
        {
            std::filesystem::path cacheDirectory = getCacheDirectory();
            if (!std::filesystem::exists(cacheDirectory))
            {
                std::filesystem::create_directories(cacheDirectory);
            }
        }

        static shaderc_shader_kind vulkanShaderStageToShaderc(VkShaderStageFlagBits stage)
        {
            switch (stage)
            {
                case VK_SHADER_STAGE_VERTEX_BIT: { return shaderc_glsl_vertex_shader; }
                case VK_SHADER_STAGE_FRAGMENT_BIT: { return shaderc_glsl_fragment_shader; }
            }

            AST_CORE_ASSERT(false, "Unknown shader stage");
            return static_cast<shaderc_shader_kind>(0);
        }

        static std::string vulkanShaderStageToString(VkShaderStageFlagBits stage)
        {
            switch (stage)
            {
                case VK_SHADER_STAGE_VERTEX_BIT: { return "vertex"; }
                case VK_SHADER_STAGE_FRAGMENT_BIT: { return "fragment"; }
            }

            AST_CORE_ASSERT(false, "Unknown shader stage");
            return "";
        }

        static std::string vulkanShaderStageCachedVulkanFileExtension(VkShaderStageFlagBits stage)
        {
            switch (stage)
            {
                case VK_SHADER_STAGE_VERTEX_BIT: { return ".vert.spv"; }
                case VK_SHADER_STAGE_FRAGMENT_BIT: { return ".frag.spv"; }
            }

            AST_CORE_ASSERT(false, "Unknown shader stage");
            return "";
        }

        static std::string extractNameFromFilepath(const std::filesystem::path& filepath)
        {
            std::string filepathStr = filepath.string();
            size_t lastSlash = filepathStr.find_last_of("/\\");
            lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
            size_t lastDot = filepathStr.rfind('.');
            size_t count = lastDot == std::string::npos ? filepathStr.size() - lastSlash : lastDot - lastSlash;

            std::string name = filepathStr.substr(lastSlash, count);
            return name;
        }
    }

    VulkanShaderCompiler::VulkanShaderCompiler(const std::filesystem::path& shaderFilepath)
        : m_ShaderFilepath(shaderFilepath)
    {
    }

    Ref<VulkanShader> VulkanShaderCompiler::compile(const std::filesystem::path& shaderFilepath)
    {
        Ref<VulkanShader> shader = Ref<VulkanShader>::create();
        shader->m_Filepath = shaderFilepath;
        shader->m_Name = Utils::extractNameFromFilepath(shaderFilepath);

        Ref<VulkanShaderCompiler> compiler = Ref<VulkanShaderCompiler>::create(shaderFilepath);
        AST_CORE_TRACE("[VulkanShaderCompiler] Processing {0}...", shaderFilepath.string());
        compiler->process();

        shader->createShaders(compiler->getShaderData());

        // [TODO] Extract from reflection data.
        ShaderDescriptorSetInfo sdsi;
        sdsi.uniformBufferInfos = {
            { 0, { 1, VK_SHADER_STAGE_VERTEX_BIT, "u_Camera" } },
        };
        sdsi.imageSamplerInfos = {
            { 1, { 32, VK_SHADER_STAGE_FRAGMENT_BIT, "u_Textures" }}
        };
        shader->setDescriptorSetInfo(sdsi);

        shader->createDescriptorSetLayouts();

        return shader;
    }

    void VulkanShaderCompiler::process()
    {
        Utils::createCacheDirectoryIfNeeded();

        std::string srcCode = readFile(m_ShaderFilepath);
        auto shaderSources = parseShader(srcCode);

        {
            Timer timer;
            compileOrGetVulkanBinaries(shaderSources);
            AST_CORE_TRACE("[VulkanShaderCompiler] Processing took {0} ms.", timer.getElapsedMilliseconds());
        }
    }

    std::string VulkanShaderCompiler::readFile(const std::filesystem::path& filepath)
    {
        std::string srcCode;

        std::ifstream in(filepath, std::ios::ate | std::ios::binary);
        AST_CORE_ASSERT(in.is_open(), "Failed to open file: {0}", filepath.string());

        in.seekg(0, std::ios::end);
        srcCode.resize(in.tellg());
        in.seekg(0, std::ios::beg);

        in.read(srcCode.data(), srcCode.size());
        in.close();

        return srcCode;
    }

    std::map<VkShaderStageFlagBits, std::string> VulkanShaderCompiler::parseShader(const std::string& srcCode)
    {
        std::map<VkShaderStageFlagBits, std::string> shaderSources;

        const char* typeToken = "#type";
        size_t typeTokenLength = strlen(typeToken);
        size_t pos = srcCode.find(typeToken, 0);

        while (pos != std::string::npos)
        {
            size_t eol = srcCode.find_first_of("\r\n", pos);
            AST_CORE_ASSERT(eol != std::string::npos, "Syntax error");

            size_t begin = pos + typeTokenLength + 1;
            std::string type = srcCode.substr(begin, eol - begin);
            AST_CORE_ASSERT(Utils::shaderTypeFromString(type), "Invalid shader type specified");


            size_t nextLinePos = srcCode.find_first_not_of("\r\n", eol);
            pos = srcCode.find(typeToken, nextLinePos);

            shaderSources[Utils::shaderTypeFromString(type)] = srcCode.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? srcCode.size() - 1 : nextLinePos));
        }

        return shaderSources;
    }

    void VulkanShaderCompiler::compileOrGetVulkanBinaries(const std::map<VkShaderStageFlagBits, std::string>& shaderSources)
    {
        shaderc::Compiler compiler;

        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        const bool optimize = true;
        if (optimize)
        {
            options.SetOptimizationLevel(shaderc_optimization_level_performance);
        }

        std::filesystem::path cacheDirectory = Utils::getCacheDirectory();

        m_ShaderData.clear();

        for (auto& [stage, source] : shaderSources)
        {
            std::filesystem::path cachePath = cacheDirectory / (m_ShaderFilepath.filename().string() + Utils::vulkanShaderStageCachedVulkanFileExtension(stage));

            std::ifstream in(cachePath, std::ios::in | std::ios::binary);
            if (in.is_open())
            {
                AST_CORE_DEBUG("Found cache file for {0} shader.", Utils::vulkanShaderStageToString(stage));

                in.seekg(0, std::ios::end);
                auto size = in.tellg();
                in.seekg(0, std::ios::beg);

                auto& data = m_ShaderData[stage];
                data.resize(size / sizeof(uint32_t));
                in.read(reinterpret_cast<char*>(data.data()), size);
            } else
            {
                AST_CORE_DEBUG("Compiling {0} shader...", Utils::vulkanShaderStageToString(stage));

                shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
                    source, Utils::vulkanShaderStageToShaderc(stage), m_ShaderFilepath.string().c_str(), options);

                if (result.GetCompilationStatus() != shaderc_compilation_status_success)
                {
                    AST_CORE_ERROR("Compilation failed: {0}", result.GetErrorMessage());
                    AST_CORE_ASSERT(false, "")
                }

                // Copy the binary data to the shader data.
                m_ShaderData[stage] = std::vector<uint32_t>(result.cbegin(), result.cend());

                // Write the binary data to the cache file.
                std::ofstream out(cachePath, std::ios::out | std::ios::binary);
                if (out.is_open())
                {
                    auto& data = m_ShaderData[stage];
                    out.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(uint32_t));
                    out.flush();
                    out.close();
                }
            }
        }

        for (auto&& [stage, data] : m_ShaderData)
        {
            reflect(stage, data);
        }
    }

    void VulkanShaderCompiler::reflect(VkShaderStageFlagBits stage, const std::vector<uint32_t>& spirv)
    {
        spirv_cross::Compiler compiler(spirv);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        AST_CORE_TRACE("[VulkanShaderCompiler::reflect] {0} - {1}", m_ShaderFilepath.string(), Utils::vulkanShaderStageToString(stage));

        AST_CORE_TRACE("Uniform buffers:");
        for (auto& resource : resources.uniform_buffers)
        {
            const spirv_cross::SPIRType& bufferType = compiler.get_type(resource.base_type_id);
            size_t bufferSize = compiler.get_declared_struct_size(bufferType);
            uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            size_t memberCount = bufferType.member_types.size();

            AST_CORE_TRACE("    {0}", resource.name);
            AST_CORE_TRACE("        Size = {0}", bufferSize);
            AST_CORE_TRACE("        Binding = {0}", binding);
            AST_CORE_TRACE("        Member count = {0}", memberCount);
        }

        AST_CORE_TRACE("Sampled images:");
        for (auto& resource : resources.sampled_images)
        {
            uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

            AST_CORE_TRACE("    {0}", resource.name);
            AST_CORE_TRACE("        Binding = {0}", binding);
            AST_CORE_TRACE("        Array size = {0}", compiler.get_type(resource.base_type_id).array.size());
        }
    }
}


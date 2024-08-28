#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanShader.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

#include "shaderc/glslc/src/file_compiler.h"
#include "spirv_cross/spirv_cross.hpp"

#include "Astranox/core/Timer.hpp"

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
    }

    void VulkanShader::bind()
    {
    }

    void VulkanShader::unbind()
    {
    }

    std::string VulkanShader::readFile(const std::filesystem::path& filepath)
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

    VulkanShader::VulkanShader(const std::filesystem::path& filepath)
        : m_Filepath(filepath)
    {
        Utils::createCacheDirectoryIfNeeded();

        std::string srcCode = readFile(filepath);
        auto shaderSources = parseShader(srcCode);

        {
            Timer timer;
            compileOrGetVulkanBinaries(shaderSources);
            AST_CORE_TRACE("VulkanShader took {0} ms", timer.getElapsedMilliseconds());
        }

        // Extract name from filepath
        std::string filepathStr = filepath.string();
        size_t lastSlash = filepathStr.find_last_of("/\\");
        lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
        size_t lastDot = filepathStr.rfind('.');
        size_t count = lastDot == std::string::npos ? filepathStr.size() - lastSlash : lastDot - lastSlash;
        m_Name = filepathStr.substr(lastSlash, count);

        createShaders(m_ShaderData);
    }

    VulkanShader::~VulkanShader()
    {
        destroy();
    }

    void VulkanShader::destroy()
    {
        auto device = VulkanContext::get()->getDevice();

        for (auto layout : m_DescriptorSetLayouts)
        {
            ::vkDestroyDescriptorSetLayout(device->getRaw(), layout, nullptr);
        }

        for (auto stage : m_ShaderStages)
        {
            ::vkDestroyShaderModule(device->getRaw(), stage.module, nullptr);
        }
    }

    void VulkanShader::createShaders(const std::map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData)
    {
        auto device = VulkanContext::get()->getDevice();

        m_ShaderStages.clear();

        for (auto& [stage, data] : shaderData)
        {
            VkShaderModuleCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = data.size() * sizeof(uint32_t),
                .pCode = data.data(),
            };

            VkShaderModule shaderModule;
            VK_CHECK(::vkCreateShaderModule(device->getRaw(), &createInfo, nullptr, &shaderModule));

            VkPipelineShaderStageCreateInfo shaderStage{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = stage,
                .module = shaderModule,
                .pName = "main",
                .pSpecializationInfo = nullptr,
            };
            m_ShaderStages.push_back(shaderStage);
        }
    }

    void VulkanShader::createDescriptorSetLayouts()
    {
        uint32_t setCount = 1;
        m_DescriptorSetLayouts.clear();
        m_DescriptorSetLayouts.resize(setCount);

        for (uint32_t setIndex = 0; setIndex < setCount; ++setIndex)
        {
            std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
            for (auto& [binding, uniformBuffer] : m_ShaderDescriptorSetInfo.uniformBufferInfos)
            {
                VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

                VkDescriptorSetLayoutBinding& bindingInfo = layoutBindings.emplace_back();
                bindingInfo = {
                    .binding = binding,
                    .descriptorType = descriptorType,
                    .descriptorCount = 1,
                    .stageFlags = uniformBuffer.shaderStage,
                    .pImmutableSamplers = nullptr,
                };

                VkWriteDescriptorSet& writeDescriptorSet = m_ShaderDescriptorSetInfo.writeDescriptorSets[uniformBuffer.name];
                writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet.dstBinding = binding;
                writeDescriptorSet.dstArrayElement = 0;
                writeDescriptorSet.descriptorCount = 1;
                writeDescriptorSet.descriptorType = descriptorType;
                writeDescriptorSet.pImageInfo = nullptr;
                writeDescriptorSet.pTexelBufferView = nullptr;
            }

            for (auto& [binding, imageSampler] : m_ShaderDescriptorSetInfo.imageSamplerInfos)
            {
                VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

                VkDescriptorSetLayoutBinding& bindingInfo = layoutBindings.emplace_back();
                bindingInfo = {
                    .binding = binding,
                    .descriptorType = descriptorType,
                    .descriptorCount = imageSampler.arraySize,
                    .stageFlags = imageSampler.shaderStage,
                    .pImmutableSamplers = nullptr,
                };

                VkWriteDescriptorSet& writeDescriptorSet = m_ShaderDescriptorSetInfo.writeDescriptorSets[imageSampler.name];
                writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet.dstBinding = binding;
                writeDescriptorSet.dstArrayElement = 0;
                writeDescriptorSet.descriptorCount = imageSampler.arraySize;
                writeDescriptorSet.descriptorType = descriptorType;
                writeDescriptorSet.pImageInfo = nullptr;
                writeDescriptorSet.pTexelBufferView = nullptr;
            }

            VkDescriptorSetLayoutCreateInfo layoutInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
                .pBindings = layoutBindings.data(),
            };

            VK_CHECK(::vkCreateDescriptorSetLayout(
                VulkanContext::get()->getDevice()->getRaw(),
                &layoutInfo,
                nullptr,
                &m_DescriptorSetLayouts[setIndex]));
        }
    }

    std::map<VkShaderStageFlagBits, std::string> VulkanShader::parseShader(const std::string& srcCode)
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

    void VulkanShader::compileOrGetVulkanBinaries(const std::map<VkShaderStageFlagBits, std::string>& shaderSources)
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
            std::filesystem::path shaderFilepath = m_Filepath;
            std::filesystem::path cachePath = cacheDirectory / (shaderFilepath.filename().string() + Utils::vulkanShaderStageCachedVulkanFileExtension(stage));

            std::ifstream in(cachePath, std::ios::in | std::ios::binary);
            if (in.is_open())
            {
                AST_CORE_INFO("Cache file found for {0} shader of {1}. Reading binaries...", Utils::vulkanShaderStageToString(stage), m_Filepath.string());

                in.seekg(0, std::ios::end);
                auto size = in.tellg();
                in.seekg(0, std::ios::beg);

                auto& data = m_ShaderData[stage];
                data.resize(size / sizeof(uint32_t));
                in.read(reinterpret_cast<char*>(data.data()), size);
            } else
            {
                AST_CORE_INFO("No cache file found for {0} shader of {1}. Compiling shader...", Utils::vulkanShaderStageToString(stage), m_Filepath.string());

                shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
                    source, Utils::vulkanShaderStageToShaderc(stage), m_Filepath.string().c_str(), options);

                if (result.GetCompilationStatus() != shaderc_compilation_status_success)
                {
                    AST_CORE_ERROR("Failed to compile shader: {0}", result.GetErrorMessage());
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

            for (auto&& [stage, data] : m_ShaderData)
            {
                reflect(stage, data);
            }
        }
    }

    void VulkanShader::reflect(VkShaderStageFlagBits stage, const std::vector<uint32_t>& shaderData)
    {
        spirv_cross::Compiler compiler(shaderData);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        AST_CORE_TRACE("VulkanShader::reflect - [{0}] {1}", Utils::vulkanShaderStageToString(stage), m_Filepath.string());

        AST_CORE_TRACE("Uniform buffers:");
        for (auto& resource : resources.uniform_buffers)
        {
            const spirv_cross::SPIRType& bufferType = compiler.get_type(resource.base_type_id);
            uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
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

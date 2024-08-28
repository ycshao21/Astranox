#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanDescriptorManager.hpp"

#include "Astranox/platform/vulkan/VulkanUniformBufferArray.hpp"
#include "Astranox/platform/vulkan/VulkanUniformBuffer.hpp"
#include "Astranox/platform/vulkan/VulkanShader.hpp"
#include "Astranox/platform/vulkan/VulkanTexture2D.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

#include "Astranox/rendering/Renderer.hpp"

namespace Astranox
{
    static RenderPassResourceType VulkanDescriptorTypeToRenderPassResourceType(VkDescriptorType type)
    {
        switch (type)
        {
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                return RenderPassResourceType::UniformBuffer;
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                return RenderPassResourceType::Texture2D;
        }

        AST_CORE_ASSERT(false, "Unknown descriptor type");
        return RenderPassResourceType::None;
    }

    static RenderPassInputType VulkanDescriptorTypeToRenderPassInputType(VkDescriptorType type)
    {
        switch (type)
        {
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                return RenderPassInputType::UniformBuffer;
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                return RenderPassInputType::ImageSampler2D;
        }

        AST_CORE_ASSERT(false, "Unknown descriptor type");
        return RenderPassInputType::None;
    }


    VulkanDescriptorManager::VulkanDescriptorManager(Ref<Shader> shader)
        : m_Shader(shader)
    {
        uint32_t framesInFlight = Renderer::getConfig().framesInFlight;
        m_WriteDescriptorMap.resize(framesInFlight);

        auto& descriptorSetInfo = shader.as<VulkanShader>()->getDescriptorSetInfo();

        uint32_t set = 0;  // [TODO] Support multiple descriptor sets
        for (auto&& [name, wd] : descriptorSetInfo.writeDescriptorSets)
        {
            uint32_t binding = wd.dstBinding;

            RenderPassInputDeclaration& inputDecl = m_RenderPassInputDeclarations[name];
            inputDecl.set = set;
            inputDecl.binding = binding;
            inputDecl.count = wd.descriptorCount;
            inputDecl.name = name;
            inputDecl.type = VulkanDescriptorTypeToRenderPassInputType(wd.descriptorType);

            RenderPassInput& input = m_RenderPassInputResources[set][binding];
            input.input.resize(wd.descriptorCount);
            input.type = VulkanDescriptorTypeToRenderPassResourceType(wd.descriptorType);

            if (inputDecl.type == RenderPassInputType::ImageSampler2D)
            {
                for (auto& texture: input.input)
                {
                    texture = Renderer::getWhiteTexture();
                }
            }

            for (uint32_t frameIndex = 0; frameIndex < framesInFlight; ++frameIndex)
            {
                m_WriteDescriptorMap[frameIndex][set][binding] = wd;
            }
        }

        // Descriptor pool >>>
        std::vector<VkDescriptorPoolSize> poolSizes{
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },            // Uniform buffer
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },    // Uniform buffer dynamic
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },    // Image sampler
        };

        VkDescriptorPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = 1000,
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data()
        };

        VkDevice device = VulkanContext::get()->getDevice()->getRaw();
        VK_CHECK(::vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool));
        // <<< Descriptor pool
    }

    VulkanDescriptorManager::~VulkanDescriptorManager()
    {
        VkDevice device = VulkanContext::get()->getDevice()->getRaw();
        ::vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
    }

    void VulkanDescriptorManager::upload()
    {
        auto device = VulkanContext::get()->getDevice();
        uint32_t framesInFlight = Renderer::getConfig().framesInFlight;

        m_DescriptorSets.clear();
        m_DescriptorSets.resize(framesInFlight);

        for (const auto& [set, inputs] : m_RenderPassInputResources)
        {
            auto layout = m_Shader.as<VulkanShader>()->getDescriptorSetLayout(set);

            for (uint32_t frameIndex = 0; frameIndex < framesInFlight; ++frameIndex)
            {
                // Descriptor set >>>
                VkDescriptorSetAllocateInfo allocInfo{
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                    .descriptorPool = m_DescriptorPool,
                    .descriptorSetCount = 1,
                    .pSetLayouts = &layout
                };

                VkDescriptorSet& descriptorSet = m_DescriptorSets[frameIndex].emplace_back();
                VK_CHECK(::vkAllocateDescriptorSets(device->getRaw(), &allocInfo, &descriptorSet));
                // <<< Descriptor set

                // Descriptor writes >>>
                auto& writeDescriptorMap = m_WriteDescriptorMap.at(frameIndex).at(set);
                std::vector<std::vector<VkDescriptorImageInfo>> imageInfos;
                uint32_t imageInfoIndex = 0;

                for (const auto& [binding, input] : inputs)
                {
                    auto& wd = writeDescriptorMap.at(binding);
                    wd.dstSet = descriptorSet;

                    switch (input.type)
                    {
                    case RenderPassResourceType::UniformBuffer:
                    {
                        auto ub = input.input[0].as<VulkanUniformBuffer>();
                        wd.pBufferInfo = &ub->getDescriptorBufferInfo();
                        break;
                    }
                    case RenderPassResourceType::UniformBufferArray:
                    {
                        auto uba = input.input[0].as<VulkanUniformBufferArray>();
                        auto ub = uba->getCurrentBuffer().as<VulkanUniformBuffer>();
                        wd.pBufferInfo = &ub->getDescriptorBufferInfo();
                        break;
                    }
                    case RenderPassResourceType::Texture2D:
                    {
                        imageInfos.emplace_back(input.input.size());
                        for (uint32_t i = 0; i < input.input.size(); ++i)
                        {
                            auto texture = input.input[i].as<VulkanTexture2D>();
                            imageInfos[imageInfoIndex][i] = texture->getDescriptorImageInfo();
                        }
                        wd.pImageInfo = imageInfos[imageInfoIndex].data();
                        ++imageInfoIndex;
                        break;
                    }
                    }

                }
                std::vector<VkWriteDescriptorSet> writeDescriptors;
                for (auto&& [binding, wd]: writeDescriptorMap)
                {
                    writeDescriptors.push_back(wd);
                }
                
                if (!writeDescriptors.empty())
                {
                    ::vkUpdateDescriptorSets(
                        device->getRaw(),
                        static_cast<uint32_t>(writeDescriptors.size()),
                        writeDescriptors.data(),
                        0,
                        nullptr);
                }
            }
        }
        
    }

    const RenderPassInputDeclaration* VulkanDescriptorManager::getRenderPassInputDeclaration(const std::string& name) const
    {
        if (m_RenderPassInputDeclarations.find(name) == m_RenderPassInputDeclarations.end())
        {
            return nullptr;
        }
        return &m_RenderPassInputDeclarations.at(name);
    }

    void VulkanDescriptorManager::setInput(const std::string& name, Ref<UniformBuffer> ub)
    {
        const RenderPassInputDeclaration* declaration = getRenderPassInputDeclaration(name);
        if (!declaration)
        {
            AST_CORE_ASSERT(false, "Render pass input {0} not found", name);
            return;
        }
        m_RenderPassInputResources.at(declaration->set).at(declaration->binding).setInput(ub, 0);
    }

    void VulkanDescriptorManager::setInput(const std::string& name, Ref<UniformBufferArray> uba)
    {
        const RenderPassInputDeclaration* declaration = getRenderPassInputDeclaration(name);
        if (!declaration)
        {
            AST_CORE_ASSERT(false, "Render pass input {0} not found", name);
            return;
        }
        m_RenderPassInputResources.at(declaration->set).at(declaration->binding).setInput(uba, 0);
    }

    void VulkanDescriptorManager::setInput(const std::string& name, Ref<Texture2D> texture, uint32_t index)
    {
        const RenderPassInputDeclaration* declaration = getRenderPassInputDeclaration(name);
        if (!declaration)
        {
            AST_CORE_ASSERT(false, "Render pass input {0} not found", name);
            return;
        }
        m_RenderPassInputResources.at(declaration->set).at(declaration->binding).setInput(texture, index);
    }
}

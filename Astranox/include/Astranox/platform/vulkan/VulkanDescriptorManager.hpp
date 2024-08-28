#pragma once

#include <vulkan/vulkan.h>
#include "Astranox/core/RefCounted.hpp"

#include "Astranox/rendering/UniformBufferArray.hpp"
#include "Astranox/rendering/Shader.hpp"
#include "Astranox/rendering/Texture2D.hpp"

namespace Astranox
{
    enum class RenderPassResourceType: uint8_t
    {
        None = 0,
        UniformBuffer,
        UniformBufferArray,
        Texture2D
    };

    enum class RenderPassInputType: uint8_t
    {
        None = 0,
        UniformBuffer,
        ImageSampler2D
    };

    struct RenderPassInput
    {
        RenderPassResourceType type = RenderPassResourceType::None;
        std::vector<Ref<RefCounted>> input;

        RenderPassInput() = default;

        RenderPassInput(Ref<UniformBuffer> ub)
            : type(RenderPassResourceType::UniformBuffer)
        {
            input.push_back(ub);
        }

        RenderPassInput(Ref<UniformBufferArray> uba)
            : type(RenderPassResourceType::UniformBufferArray)
        {
            input.push_back(uba);
        }

        RenderPassInput(Ref<Texture2D> texture)
            : type(RenderPassResourceType::Texture2D)
        {
            input.push_back(texture);
        }

        void setInput(Ref<UniformBuffer> ub, uint32_t index)
        {
            type = RenderPassResourceType::UniformBuffer;
            input[index] = ub;
        }

        void setInput(Ref<UniformBufferArray> uba, uint32_t index)
        {
            type = RenderPassResourceType::UniformBufferArray;
            input[index] = uba;
        }

        void setInput(Ref<Texture2D> texture, uint32_t index)
        {
            type = RenderPassResourceType::Texture2D;
            input[index] = texture;
        }
    };

    struct RenderPassInputDeclaration
    {
        RenderPassInputType type = RenderPassInputType::None;
        uint32_t set = 0;
        uint32_t binding = 0;
        uint32_t count = 0;
        std::string name;
    };

    class VulkanDescriptorManager: public RefCounted
    {
    public:
        VulkanDescriptorManager(Ref<Shader> shader);
        virtual ~VulkanDescriptorManager();

        void upload();

        VkDescriptorPool getDescriptorPool() const { return m_DescriptorPool; }
        const std::vector<VkDescriptorSet>& getDescriptorSets(uint32_t frameIndex) const { 
            return m_DescriptorSets[frameIndex];
        }

        const RenderPassInputDeclaration* getRenderPassInputDeclaration(const std::string& name) const;

        void setInput(const std::string& name, Ref<UniformBuffer> ub);
        void setInput(const std::string& name, Ref<UniformBufferArray> uba);
        void setInput(const std::string& name, Ref<Texture2D> texture, uint32_t index);

    private:
        std::map<uint32_t, std::map<uint32_t, RenderPassInput>> m_RenderPassInputResources;  // [set, [binding, input]]
        std::map<std::string, RenderPassInputDeclaration> m_RenderPassInputDeclarations;
        std::vector<std::map<uint32_t, std::map<uint32_t, VkWriteDescriptorSet>>> m_WriteDescriptorMap;  // [frame, [set, [binding, wd]]]

        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
        std::vector<std::vector<VkDescriptorSet>> m_DescriptorSets;

        Ref<Shader> m_Shader = nullptr;
    };
}

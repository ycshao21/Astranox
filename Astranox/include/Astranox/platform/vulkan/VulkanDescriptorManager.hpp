#pragma once

#include <vulkan/vulkan.h>
#include "Astranox/core/RefCounted.hpp"

#include "Astranox/rendering/UniformBufferArray.hpp"
#include "Astranox/rendering/Shader.hpp"

namespace Astranox
{
    class VulkanDescriptorManager: public RefCounted
    {
    public:
        VulkanDescriptorManager();
        virtual ~VulkanDescriptorManager();

        //void init(VkDescriptorSetLayout layout, VkSampler sampler, VkImageView imageView, Ref<UniformBufferArray> uba);
        void createDescriptorSets(Ref<Shader> shader, Ref<UniformBufferArray> uba);

        VkDescriptorPool getDescriptorPool() const { return m_DescriptorPool; }
        const std::vector<VkDescriptorSet>& getDescriptorSets(uint32_t frameIndex) const { 
            return m_DescriptorSets[frameIndex];
        }

    private:
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
        std::vector<std::vector<VkDescriptorSet>> m_DescriptorSets;
    };
}

#pragma once

#include <vulkan/vulkan.h>
#include "Astranox/core/RefCounted.hpp"

#include "Astranox/rendering/UniformBufferArray.hpp"

namespace Astranox
{
    class VulkanDescriptorManager: public RefCounted
    {
    public:
        VulkanDescriptorManager();
        virtual ~VulkanDescriptorManager();

        void init(VkDescriptorSetLayout layout, VkSampler sampler, VkImageView imageView, Ref<UniformBufferArray> uba);

        const std::vector<VkDescriptorSet>& getDescriptorSets() const { return m_DescriptorSets; }

    private:
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_DescriptorSets;
    };
}

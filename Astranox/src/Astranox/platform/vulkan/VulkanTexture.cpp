#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanTexture.hpp"

#include "Astranox/platform/vulkan/VulkanMemoryAllocator.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

#include <stb_image/stb_image.h>

namespace Astranox
{
    VulkanTexture::VulkanTexture(const std::filesystem::path& path, bool enableMipmaps)
        : m_Path(path)
    {
        loadFromFile(path);

        m_TextureMipLevels = enableMipmaps ? calculateMipLevels() : 1;

        VkDevice device = VulkanContext::get()->getDevice()->getRaw();
        {
            // Texture image >>>
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            VulkanMemoryAllocator::createBuffer(
                m_Buffer.size,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer,
                stagingBufferMemory
            );

            void* data;
            VK_CHECK(::vkMapMemory(device, stagingBufferMemory, 0, m_Buffer.size, 0, &data));
            std::memcpy(data, m_Buffer.data, static_cast<size_t>(m_Buffer.size));
            ::vkUnmapMemory(device, stagingBufferMemory);

            VkImageUsageFlags textureUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                  VK_IMAGE_USAGE_SAMPLED_BIT;

            VulkanMemoryAllocator::createImage(
                m_Width,
                m_Height,
                m_TextureMipLevels,
                VK_SAMPLE_COUNT_1_BIT,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                textureUsageFlags,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_TextureImage,
                m_TextureImageMemory
            );

            VulkanMemoryAllocator::transitionImageLayout(
                m_TextureImage,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                m_TextureMipLevels
            );

            VulkanMemoryAllocator::copyBufferToImage(
                stagingBuffer,
                m_TextureImage,
                m_Width,
                m_Height
            );

            //transitionImageLayout(
            //    m_TextureImage,
            //    VK_FORMAT_R8G8B8A8_SRGB,
            //    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            //    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            //);

            generateMipmaps();

            VulkanMemoryAllocator::destroyBuffer(stagingBuffer, stagingBufferMemory);
            // <<< Texture image

            // Texture image view >>>
            m_TextureImageView = createImageView(
                m_TextureImage,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_ASPECT_COLOR_BIT,
                m_TextureMipLevels
            );
            // <<< Texture image view

            // Texture sampler >>>
            auto device = VulkanContext::get()->getDevice();

            VkSamplerCreateInfo samplerInfo{
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .mipLodBias = 0.0f,
                .anisotropyEnable = VK_TRUE,
                .maxAnisotropy = device->getPhysicalDevice()->getProperties().limits.maxSamplerAnisotropy,
                .compareEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_ALWAYS,
                .minLod = 0,
                .maxLod = VK_LOD_CLAMP_NONE,
                .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                .unnormalizedCoordinates = VK_FALSE,
            };
            VK_CHECK(::vkCreateSampler(device->getRaw(), &samplerInfo, nullptr, &m_TextureSampler));
            // <<< Texture sampler
        }

        stbi_image_free(m_Buffer.data);
    }

    VulkanTexture::~VulkanTexture()
    {
        VkDevice device = VulkanContext::get()->getDevice()->getRaw();

        ::vkDestroySampler(device, m_TextureSampler, nullptr);
        ::vkDestroyImageView(device, m_TextureImageView, nullptr);
        ::vkDestroyImage(device, m_TextureImage, nullptr);
        ::vkFreeMemory(device, m_TextureImageMemory, nullptr);
    }

    void VulkanTexture::loadFromFile(const std::filesystem::path& path)
    {
        std::string pathStr = path.string();

        int texWidth, texHeight, texChannels;
        m_Buffer.data = stbi_load(pathStr.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        AST_CORE_ASSERT(m_Buffer.data, "Failed to load texture image");

        m_Buffer.size = texWidth * texHeight * 4;
        
        m_Width = texWidth;
        m_Height = texHeight;
        m_Channels = texChannels;
    }

    void VulkanTexture::generateMipmaps()
    {
        auto device = VulkanContext::get()->getDevice();
        auto commandPool = device->getCommandPool();

        VkCommandBuffer blitCmdBuffer = commandPool->allocateCommandBuffer();
        commandPool->beginOneTimeBuffer(blitCmdBuffer);

        VkImageMemoryBarrier barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = m_TextureImage,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        };

        int32_t mipWidth = m_Width;
        int32_t mipHeight = m_Height;

        for (uint32_t i = 1; i < m_TextureMipLevels; i++)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            ::vkCmdPipelineBarrier(
                blitCmdBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            VkImageBlit blit{
                .srcSubresource = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = i - 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                },
                .srcOffsets = {
                    { 0, 0, 0 },
                    { mipWidth, mipHeight, 1 }
                },
                .dstSubresource = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = i,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                },
                .dstOffsets = {
                    { 0, 0, 0 },
                    {
                        mipWidth > 1 ? (mipWidth >> 1) : 1,
                        mipHeight > 1 ? (mipHeight >> 1) : 1,
                        1
                    }
                }
            };

            ::vkCmdBlitImage(
                blitCmdBuffer,
                m_TextureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                m_TextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR
            );

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            ::vkCmdPipelineBarrier(
                blitCmdBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            if (mipWidth > 1) { mipWidth >>= 1; }
            if (mipHeight > 1) { mipHeight >>= 1; }
        }

        barrier.subresourceRange.baseMipLevel = m_TextureMipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        ::vkCmdPipelineBarrier(
            blitCmdBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        commandPool->endOneTimeBuffer(blitCmdBuffer);
    }

    uint32_t VulkanTexture::calculateMipLevels()
    {
        return static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;
    }

    VkImageView VulkanTexture::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
    {
        VkImageView imageView;
        VkImageViewCreateInfo viewInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = {
                .aspectMask = aspectFlags,
                .baseMipLevel = 0,
                .levelCount = mipLevels,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        auto device = VulkanContext::get()->getDevice();
        VK_CHECK(::vkCreateImageView(device->getRaw(), &viewInfo, nullptr, &imageView));
        return imageView;
    }
}


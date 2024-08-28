#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanTexture2D.hpp"

#include "Astranox/platform/vulkan/VulkanMemoryAllocator.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

#include <stb_image/stb_image.h>

namespace Astranox
{
    VulkanTexture2D::VulkanTexture2D(const std::filesystem::path& path, bool enableMipmaps)
        : m_Path(path)
    {
        loadFromFile(path);

        m_TextureMipLevels = enableMipmaps ? calculateMipLevels() : 1;

        auto device = VulkanContext::get()->getDevice();
        VulkanMemoryAllocator allocator("VulkanTexture");

        // Texture image >>>
        VkBufferCreateInfo stagingBufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = m_Buffer.size,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation = allocator.createBuffer(
            stagingBufferCI,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            stagingBuffer
        );

        void* dest = allocator.mapMemory<void>(stagingBufferAllocation);
        std::memcpy(dest, m_Buffer.data, static_cast<size_t>(m_Buffer.size));
        allocator.unmapMemory(stagingBufferAllocation);

        VkImageCreateInfo imageInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .extent = {
                .width = m_Width,
                .height = m_Height,
                .depth = 1
            },
            .mipLevels = m_TextureMipLevels,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                  VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                  VK_IMAGE_USAGE_SAMPLED_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        m_TextureImageAllocation = allocator.createImage(
            imageInfo,
            VMA_MEMORY_USAGE_GPU_ONLY,
            m_TextureImage
        );

        allocator.transitionImageLayout(
            m_TextureImage,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            m_TextureMipLevels
        );

        allocator.copyBufferToImage(
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

        allocator.destroyBuffer(stagingBuffer, stagingBufferAllocation);
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

        stbi_image_free(m_Buffer.data);

        m_DescriptorImageInfo = {
            .sampler = m_TextureSampler,
            .imageView = m_TextureImageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
    }

    VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height, Buffer buffer)
        : m_Width(width), m_Height(height), m_Channels(4), m_Buffer(buffer)
    {
        m_TextureMipLevels = 1;

        auto device = VulkanContext::get()->getDevice();
        VulkanMemoryAllocator allocator("VulkanTexture");

        // Texture image >>>
        //m_Buffer.size = m_Width * m_Height * 4;
        //m_Buffer.data = new uint8_t[m_Buffer.size];
        //memcpy(m_Buffer.data, buffer.data, m_Buffer.size);

        VkBufferCreateInfo stagingBufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = m_Buffer.size,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation = allocator.createBuffer(
            stagingBufferCI,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            stagingBuffer
        );

        void* dest = allocator.mapMemory<void>(stagingBufferAllocation);
        std::memcpy(dest, m_Buffer.data, static_cast<size_t>(m_Buffer.size));
        allocator.unmapMemory(stagingBufferAllocation);

        VkImageCreateInfo imageInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .extent = {
                .width = m_Width,
                .height = m_Height,
                .depth = 1
            },
            .mipLevels = m_TextureMipLevels,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                  VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                  VK_IMAGE_USAGE_SAMPLED_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        m_TextureImageAllocation = allocator.createImage(
            imageInfo,
            VMA_MEMORY_USAGE_GPU_ONLY,
            m_TextureImage
        );

        allocator.transitionImageLayout(
            m_TextureImage,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            m_TextureMipLevels
        );

        allocator.copyBufferToImage(
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

        allocator.destroyBuffer(stagingBuffer, stagingBufferAllocation);
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

        m_DescriptorImageInfo = {
            .sampler = m_TextureSampler,
            .imageView = m_TextureImageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
    }

    VulkanTexture2D::~VulkanTexture2D()
    {
        VkDevice device = VulkanContext::get()->getDevice()->getRaw();

        VulkanMemoryAllocator allocator("VulkanTexture");
        ::vkDestroySampler(device, m_TextureSampler, nullptr);
        ::vkDestroyImageView(device, m_TextureImageView, nullptr);
        allocator.destroyImage(m_TextureImage, m_TextureImageAllocation);
    }

    void VulkanTexture2D::loadFromFile(const std::filesystem::path& path)
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

    void VulkanTexture2D::generateMipmaps()
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

    uint32_t VulkanTexture2D::calculateMipLevels()
    {
        return static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;
    }

    VkImageView VulkanTexture2D::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
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


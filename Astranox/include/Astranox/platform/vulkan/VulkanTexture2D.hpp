#pragma once
#include "Astranox/rendering/Texture2D.hpp"
#include <filesystem>
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

namespace Astranox
{
    class VulkanTexture2D: public Texture2D
    {
    public:
        VulkanTexture2D(const std::filesystem::path& path, bool enableMipmaps = true);
        VulkanTexture2D(uint32_t width = 1, uint32_t height = 1, Buffer buffer = {});
        virtual ~VulkanTexture2D();

    public:
        void loadFromFile(const std::filesystem::path& path);
        void generateMipmaps();

    public:
        uint32_t getWidth() const override { return m_Width; }
        uint32_t getHeight() const override { return m_Height; }
        uint32_t getChannels() const override { return m_Channels; }
        uint32_t getMipLevels() const override { return m_TextureMipLevels; }
        
        const std::filesystem::path& getPath() const override { return m_Path; }

        VkImage getImage() { return m_TextureImage; }
        VkImageView getImageView() { return m_TextureImageView; }
        VkSampler getSampler() { return m_TextureSampler; }

        const VkDescriptorImageInfo& getDescriptorImageInfo() const { return m_DescriptorImageInfo; }

    public:
        bool operator==(const Texture2D& other) const override
        {
            // Temp, compare handles instead
            return m_Path == other.getPath();
        }

    private:
        uint32_t calculateMipLevels();

        VkImageView createImageView(
            VkImage image,
            VkFormat format,
            VkImageAspectFlags aspectFlags,
            uint32_t mipLevels
        );

    private:
        // Image Info

        std::filesystem::path m_Path;

        uint32_t m_Width;
        uint32_t m_Height;
        uint32_t m_Channels;

        Buffer m_Buffer;

        // Vulkan Info

        uint32_t m_TextureMipLevels = 1;

        VkImage m_TextureImage;
        VmaAllocation m_TextureImageAllocation;
        VkImageView m_TextureImageView;
        VkSampler m_TextureSampler;

        VkDescriptorImageInfo m_DescriptorImageInfo{};
    }; 
}

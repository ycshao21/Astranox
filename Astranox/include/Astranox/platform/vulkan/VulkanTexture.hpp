#pragma once
#include "Astranox/rendering/Texture.hpp"
#include <filesystem>
#include <vulkan/vulkan.h>

namespace Astranox
{
    class VulkanTexture: public Texture
    {
    public:
        VulkanTexture(const std::filesystem::path& path, bool enableMipmaps = true);
        virtual ~VulkanTexture();

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
        VkDeviceMemory m_TextureImageMemory;
        VkImageView m_TextureImageView;
        VkSampler m_TextureSampler;
    }; 
}

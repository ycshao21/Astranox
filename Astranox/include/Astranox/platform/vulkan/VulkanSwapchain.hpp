#pragma once
#include "Astranox/core/RefCounted.hpp"

#include "VulkanDevice.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanShader.hpp"
#include "VulkanPipeline.hpp"

#include "Astranox/rendering/PerspectiveCamera.hpp"

#include <vector>

namespace Astranox
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texCoord;

        bool operator==(const Vertex& other) const
        {
            return position == other.position
                && color == other.color
                && texCoord == other.texCoord;
        }
    };

    class VulkanSwapchain: public RefCounted
    {
    public:
        VulkanSwapchain(Ref<VulkanDevice> device);
        virtual ~VulkanSwapchain() = default;

        void createSurface();
        void createSwapchain(uint32_t width, uint32_t height);

        void destroy();

    public:
        void resize(uint32_t width, uint32_t height);

        void drawFrame();

        void beginFrame();
        void present();

    public:
        uint32_t getWidth() const { return m_SwapchainExtent.width; }
        uint32_t getHeight() const { return m_SwapchainExtent.height; }

        VkRenderPass getVkRenderPass() { return m_RenderPass; }

    private:
        void chooseSurfaceFormat();

        void getQueueIndices();
        void getSwapchainImages();

        VkFramebuffer getCurrentFramebuffer() { return m_Framebuffers[m_CurrentImageIndex]; }
        VkCommandBuffer getCurrentCommandBuffer() { return m_CommandBuffers[m_CurrentFramebufferIndex]; }

        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();

        void createBuffer(
            VkDeviceSize bufferSize,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory
        );

        void copyBuffer(
            VkBuffer srcBuffer,
            VkBuffer dstBuffer,
            VkDeviceSize size
        );

        void createDescriptorPool();
        void createImage(
            uint32_t width,
            uint32_t height,
            uint32_t mipLevels,
            VkSampleCountFlagBits numSamples,
            VkFormat format,
            VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& imageMemory
        );
        VkImageView createImageView(
            VkImage image,
            VkFormat format,
            VkImageAspectFlags aspectFlags,
            uint32_t mipLevels
        );

        void beginOneTimeCommandBuffer(VkCommandBuffer& commandBuffer);
        void endOneTimeCommandBuffer(VkCommandBuffer commandBuffer);

        void transitionImageLayout(
            VkImage image,
            VkFormat format,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            uint32_t mipLevels
        );

        void copyBufferToImage(
            VkBuffer buffer,
            VkImage image,
            uint32_t width,
            uint32_t height
        );

        void generateMipmaps(
            VkImage image,
            int32_t texWidth,
            int32_t texHeight,
            uint32_t mipLevels
        );

        void loadModel(const std::string& modelPath);

        VkSampleCountFlagBits getMaxUsableSampleCount();
        
    private:
        Ref<VulkanDevice> m_Device = nullptr;

        uint32_t m_GraphicsQueueIndex = 0;
        uint32_t m_PresentQueueIndex = 0;

        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        VkFormat m_ImageFormat{};
        VkColorSpaceKHR m_ColorSpace{};

        VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
        VkExtent2D m_SwapchainExtent{};

        struct SwapchainImage
        {
            VkImage image;
            VkImageView imageView;
        };
        std::vector<SwapchainImage> m_Images;

        VkRenderPass m_RenderPass = VK_NULL_HANDLE;

        std::vector<VkFramebuffer> m_Framebuffers;
        uint32_t m_CurrentFramebufferIndex = 0;
        uint32_t m_CurrentImageIndex = 0;

        Ref<VulkanCommandPool> m_CommandPool = nullptr;
        uint32_t m_MaxFramesInFlight = 2;
        std::vector<VkCommandBuffer> m_CommandBuffers;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;

        // Remove
		Ref<VulkanShader> m_Shader = nullptr;
		Ref<VulkanPipeline> m_Pipeline = nullptr;

        VkImage m_ColorImage = VK_NULL_HANDLE;
        VkDeviceMemory m_ColorImageMemory = VK_NULL_HANDLE;
        VkImageView m_ColorImageView = VK_NULL_HANDLE;

        VkImage m_DepthImage = VK_NULL_HANDLE;
        VkDeviceMemory m_DepthImageMemory = VK_NULL_HANDLE;
        VkImageView m_DepthImageView = VK_NULL_HANDLE;

        uint32_t m_TextureMipLevels = 1;

        VkImage m_TextureImage;
        VkDeviceMemory m_TextureImageMemory;
        VkImageView m_TextureImageView;
        VkSampler m_TextureSampler;

        //mutable std::vector<float> vertices = {
        //     0.5f,  0.5f, 0.5f,    0.1f, 0.9f, 0.1f, 1.0f,    1.0f, 0.0f,
        //    -0.5f,  0.5f, 0.5f,    0.9f, 0.1f, 0.1f, 1.0f,    0.0f, 0.0f,
        //    -0.5f, -0.5f, 0.5f,    0.9f, 0.1f, 0.1f, 1.0f,    0.0f, 1.0f,
        //     0.5f, -0.5f, 0.5f,    0.9f, 0.9f, 0.9f, 1.0f,    1.0f, 1.0f,

        //     0.5f,  0.5f, -0.5f,    0.1f, 0.9f, 0.1f, 1.0f,    1.0f, 0.0f,
        //    -0.5f,  0.5f, -0.5f,    0.9f, 0.1f, 0.1f, 1.0f,    0.0f, 0.0f,
        //    -0.5f, -0.5f, -0.5f,    0.9f, 0.1f, 0.1f, 1.0f,    0.0f, 1.0f,
        //     0.5f, -0.5f, -0.5f,    0.9f, 0.9f, 0.9f, 1.0f,    1.0f, 1.0f,
        //};
        std::vector<Vertex> vertices;
        VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;

        //mutable std::vector<uint16_t> indices = {
        //    0, 1, 2, 2, 3, 0,
        //    4, 5, 6, 6, 7, 4,
        //};
        std::vector<uint32_t> indices;
        VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory m_IndexBufferMemory = VK_NULL_HANDLE;

        std::vector<VkBuffer> m_UniformBuffers;
        std::vector<VkDeviceMemory> m_UniformBuffersMemory;
        std::vector<void*> m_MappedUniformBuffers;

        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_DescriptorSets;

        VkSampleCountFlagBits m_MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        Ref<PerspectiveCamera> m_Camera = nullptr;
    };
}

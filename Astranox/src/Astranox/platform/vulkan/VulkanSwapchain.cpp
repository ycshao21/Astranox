#include "pch.hpp"

#include "Astranox/platform/vulkan/VulkanSwapchain.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

#include "Astranox/core/Application.hpp"

#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <chrono>

#include "stb_image/stb_image.h"
#include "tinyobjloader/tiny_obj_loader.h"

namespace std
{
    template<>
    struct hash<Astranox::Vertex>
    {
        size_t operator()(Astranox::Vertex const& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.position) ^
                (hash<glm::vec4>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

namespace Astranox
{
    struct UniformBufferObject {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    VulkanSwapchain::VulkanSwapchain(Ref<VulkanDevice> device)
        : m_Device(device)
    {
    }

    void VulkanSwapchain::createSurface()
    {
        VkInstance instance = VulkanContext::getInstance();

        Window& window = Application::get().getWindow();
        GLFWwindow* windowHandle = static_cast<GLFWwindow*>(window.getHandle());

        VK_CHECK(::glfwCreateWindowSurface(instance, windowHandle, nullptr, &m_Surface));

        getQueueIndices();
        chooseSurfaceFormat();
    }

    void VulkanSwapchain::createSwapchain(uint32_t width, uint32_t height)
    {
        auto physicalDevice = m_Device->getPhysicalDevice();

        // Choose extent >>>
        VkSurfaceCapabilitiesKHR surfaceCapabilities{};
        VK_CHECK(::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice->getRaw(), m_Surface, &surfaceCapabilities));

        if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            // [NOTE] If the surface size is defined, we use it.
            m_SwapchainExtent = surfaceCapabilities.currentExtent;
        } else {
            // [NOTE] Otherwise, we set the size to the window size.

            // [TODO] This should not be platform specific, but for now, we call GLFW function.
            GLFWwindow* windowHandle = static_cast<GLFWwindow*>(Application::get().getWindow().getHandle());
            ::glfwGetFramebufferSize(windowHandle, (int*)&width, (int*)&height);

            m_SwapchainExtent.width = std::clamp(static_cast<uint32_t>(width), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
            m_SwapchainExtent.height = std::clamp(static_cast<uint32_t>(height), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
        }

        if (m_SwapchainExtent.width == 0 || m_SwapchainExtent.height == 0) {
            AST_CORE_WARN("Attempting to create a swapchain with width or height of 0. Skipping...");
            return;
        }

        //AST_CORE_TRACE("Swapchain size: {0}x{1}", m_SwapchainExtent.width, m_SwapchainExtent.height);
        // <<< Choose extent


        // Choose present mode >>>
        uint32_t presentModeCount = 0;
        ::vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice->getRaw(), m_Surface, &presentModeCount, nullptr);
        AST_CORE_ASSERT(presentModeCount != 0, "Failed to get present modes");

        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        ::vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice->getRaw(), m_Surface, &presentModeCount, presentModes.data());

        // [NOTE] We prefer VK_PRESENT_MODE_MAILBOX_KHR, but if not available, we choose VK_PRESENT_MODE_FIFO_KHR.
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        for (const auto& mode: presentModes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                presentMode = mode;
                break;
            }
        }
        // <<< Choose present mode

        uint32_t desiredImageCount = surfaceCapabilities.minImageCount + 1;
        if (surfaceCapabilities.maxImageCount > 0 && desiredImageCount > surfaceCapabilities.maxImageCount) {
            desiredImageCount = surfaceCapabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_Surface,
            .minImageCount = desiredImageCount,
            .imageFormat = m_ImageFormat,
            .imageColorSpace = m_ColorSpace,
            .imageExtent = m_SwapchainExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = surfaceCapabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE
        };

        std::vector<uint32_t> queueFamilyIndices = { m_GraphicsQueueIndex, m_PresentQueueIndex };
        if (m_GraphicsQueueIndex == m_PresentQueueIndex) {  // We prefer exclusive mode
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
            createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        }

        if (m_Swapchain != VK_NULL_HANDLE) {
            ::vkDestroySwapchainKHR(m_Device->getRaw(), m_Swapchain, nullptr);
        }
        VK_CHECK(::vkCreateSwapchainKHR(m_Device->getRaw(), &createInfo, nullptr, &m_Swapchain));

        // Destroy old image views
        for (auto& image : m_Images)
        {
            ::vkDestroyImageView(m_Device->getRaw(), image.imageView, nullptr);
        }
        getSwapchainImages();

        if (m_DepthImage)
        {
            ::vkDestroyImageView(m_Device->getRaw(), m_DepthImageView, nullptr);
            ::vkDestroyImage(m_Device->getRaw(), m_DepthImage, nullptr);
            ::vkFreeMemory(m_Device->getRaw(), m_DepthImageMemory, nullptr);
        }
        createImage(
            m_SwapchainExtent.width,
            m_SwapchainExtent.height,
            physicalDevice->getDepthFormat(),
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_DepthImage,
            m_DepthImageMemory
        );

        m_DepthImageView = createImageView(
            m_DepthImage,
            physicalDevice->getDepthFormat(),
            VK_IMAGE_ASPECT_DEPTH_BIT
        );

        if (!m_RenderPass)
        {
            createRenderPass();
        }

        for (auto framebuffer : m_Framebuffers)
        {
            ::vkDestroyFramebuffer(m_Device->getRaw(), framebuffer, nullptr);
        }
        createFramebuffers();

        if (!m_CommandPool)
        {
            m_CommandPool = Ref<VulkanCommandPool>::create();
        }

        m_CommandBuffers = m_CommandPool->allocateCommandBuffers(m_MaxFramesInFlight);

        if (m_InFlightFences.empty())
        {
            createSyncObjects();
        }

        // Remove these

        if (!m_Shader)
        {
            std::string vertexShaderPath = "../Astranox-Rasterization/assets/shaders/square-vert.spv";
            std::string fragmentShaderPath = "../Astranox-Rasterization/assets/shaders/square-frag.spv";
            m_Shader = VulkanShader::create(vertexShaderPath, fragmentShaderPath);
            m_Shader->createDescriptorSetLayout();

            m_Pipeline = Ref<VulkanPipeline>::create(m_Shader);
            m_Pipeline->createPipeline();

            std::string modelPath = "../Astranox-Rasterization/assets/models/viking_room.obj";
            loadModel(modelPath);

            // Texture
            {
                // Texture image >>>
                std::string texturePath = "../Astranox-Rasterization/assets/textures/viking_room.png";
                int texWidth, texHeight, texChannels;
                stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
                AST_CORE_ASSERT(pixels, "Failed to load texture image");

                VkDeviceSize imageSize = texWidth * texHeight * 4;

                VkBuffer stagingBuffer;
                VkDeviceMemory stagingBufferMemory;
                createBuffer(
                    imageSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    stagingBuffer,
                    stagingBufferMemory
                );

                void* data;
                VK_CHECK(::vkMapMemory(m_Device->getRaw(), stagingBufferMemory, 0, imageSize, 0, &data));
                memcpy(data, pixels, static_cast<size_t>(imageSize));
                ::vkUnmapMemory(m_Device->getRaw(), stagingBufferMemory);

                stbi_image_free(pixels);

                createImage(
                    texWidth,
                    texHeight,
                    VK_FORMAT_R8G8B8A8_SRGB,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    m_TextureImage,
                    m_TextureImageMemory
                );

                transitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                copyBufferToImage(stagingBuffer, m_TextureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
                transitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                ::vkDestroyBuffer(m_Device->getRaw(), stagingBuffer, nullptr);
                ::vkFreeMemory(m_Device->getRaw(), stagingBufferMemory, nullptr);
                // <<< Texture image

                // Texture image view >>>
                m_TextureImageView = createImageView(
                    m_TextureImage,
                    VK_FORMAT_R8G8B8A8_SRGB,
                    VK_IMAGE_ASPECT_COLOR_BIT
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
                    .maxAnisotropy = m_Device->getPhysicalDevice()->getProperties().limits.maxSamplerAnisotropy,
                    .compareEnable = VK_FALSE,
                    .compareOp = VK_COMPARE_OP_ALWAYS,
                    .minLod = 0.0f,
                    .maxLod = 0.0f,
                    .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                    .unnormalizedCoordinates = VK_FALSE,
                };
                VK_CHECK(::vkCreateSampler(m_Device->getRaw(), &samplerInfo, nullptr, &m_TextureSampler));
                // <<< Texture sampler
            }

            // Vertex buffer
            {
                VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

                VkBuffer stagingBuffer;
                VkDeviceMemory stagingBufferMemory;
                createBuffer(
                    bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    stagingBuffer,
                    stagingBufferMemory
                );

                // Upload data to staging buffer
                void* data;
                VK_CHECK(::vkMapMemory(m_Device->getRaw(), stagingBufferMemory, 0, bufferSize, 0, &data));
                memcpy(data, vertices.data(), bufferSize);
                ::vkUnmapMemory(m_Device->getRaw(), stagingBufferMemory);

                createBuffer(
                    bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    m_VertexBuffer,
                    m_VertexBufferMemory
                );

                copyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

                ::vkDestroyBuffer(m_Device->getRaw(), stagingBuffer, nullptr);
                ::vkFreeMemory(m_Device->getRaw(), stagingBufferMemory, nullptr);
            }

            // Index buffer
            {
                VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

                VkBuffer stagingBuffer;
                VkDeviceMemory stagingBufferMemory;
                createBuffer(
                    bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    stagingBuffer,
                    stagingBufferMemory
                );

                // Upload data to staging buffer
                void* data;
                VK_CHECK(::vkMapMemory(m_Device->getRaw(), stagingBufferMemory, 0, bufferSize, 0, &data));
                memcpy(data, indices.data(), bufferSize);
                ::vkUnmapMemory(m_Device->getRaw(), stagingBufferMemory);

                createBuffer(
                    bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    m_IndexBuffer,
                    m_IndexBufferMemory
                );

                copyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

                ::vkDestroyBuffer(m_Device->getRaw(), stagingBuffer, nullptr);
                ::vkFreeMemory(m_Device->getRaw(), stagingBufferMemory, nullptr);
            }

            // Uniform buffers
            {
                m_UniformBuffers.resize(m_MaxFramesInFlight);
                m_UniformBuffersMemory.resize(m_MaxFramesInFlight);
                m_MappedUniformBuffers.resize(m_MaxFramesInFlight);

                VkDeviceSize bufferSize = sizeof(UniformBufferObject);
                for (size_t i = 0; i < m_MaxFramesInFlight; i++)
                {
                    createBuffer(
                        bufferSize,
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        m_UniformBuffers[i],
                        m_UniformBuffersMemory[i]
                    );

                    VK_CHECK(::vkMapMemory(m_Device->getRaw(), m_UniformBuffersMemory[i], 0, bufferSize, 0, &m_MappedUniformBuffers[i]));
                }
            }

            createDescriptorPool();

            // Descriptor sets >>>
            std::vector<VkDescriptorSetLayout> layouts(m_MaxFramesInFlight, m_Shader->getDescriptorSetLayouts()[0]);
            VkDescriptorSetAllocateInfo allocInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = m_DescriptorPool,
                .descriptorSetCount = static_cast<uint32_t>(m_MaxFramesInFlight),
                .pSetLayouts = layouts.data()
            };

            m_DescriptorSets.resize(m_MaxFramesInFlight);
            VK_CHECK(::vkAllocateDescriptorSets(m_Device->getRaw(), &allocInfo, m_DescriptorSets.data()));

            for (size_t i = 0; i < m_MaxFramesInFlight; i++)
            {
                // Uniform buffer
                VkDescriptorBufferInfo bufferInfo{
                    .buffer = m_UniformBuffers[i],
                    .offset = 0,
                    .range = sizeof(UniformBufferObject)
                };

                // Sampler
                VkDescriptorImageInfo imageInfo{
                    .sampler = m_TextureSampler,
                    .imageView = m_TextureImageView,
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                };

                std::vector<VkWriteDescriptorSet> descriptorWrites{
                    // Uniform buffer
                    {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet = m_DescriptorSets[i],
                        .dstBinding = 0,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .pImageInfo = nullptr,
                        .pBufferInfo = &bufferInfo,
                        .pTexelBufferView = nullptr
                    },
                    // Sampler
                    {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet = m_DescriptorSets[i],
                        .dstBinding = 1,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        .pImageInfo = &imageInfo,
                        .pBufferInfo = nullptr,
                        .pTexelBufferView = nullptr
                    }
                };

                ::vkUpdateDescriptorSets(
                    m_Device->getRaw(),
                    static_cast<uint32_t>(descriptorWrites.size()),
                    descriptorWrites.data(),
                    0,
                    nullptr
                );
            }
            // <<< Descriptor sets
        }
    }

    void VulkanSwapchain::destroy()
    {
        m_Device->waitIdle();

        // Remove
        ::vkDestroyBuffer(m_Device->getRaw(), m_VertexBuffer, nullptr);
        ::vkFreeMemory(m_Device->getRaw(), m_VertexBufferMemory, nullptr);

        ::vkDestroyBuffer(m_Device->getRaw(), m_IndexBuffer, nullptr);
        ::vkFreeMemory(m_Device->getRaw(), m_IndexBufferMemory, nullptr);

        for (size_t i = 0; i < m_MaxFramesInFlight; i++)
        {
            ::vkDestroyBuffer(m_Device->getRaw(), m_UniformBuffers[i], nullptr);
            ::vkFreeMemory(m_Device->getRaw(), m_UniformBuffersMemory[i], nullptr);
        }

        ::vkDestroySampler(m_Device->getRaw(), m_TextureSampler, nullptr);
        ::vkDestroyImageView(m_Device->getRaw(), m_TextureImageView, nullptr);
        ::vkDestroyImage(m_Device->getRaw(), m_TextureImage, nullptr);
        ::vkFreeMemory(m_Device->getRaw(), m_TextureImageMemory, nullptr);

        ::vkDestroyImageView(m_Device->getRaw(), m_DepthImageView, nullptr);
        ::vkDestroyImage(m_Device->getRaw(), m_DepthImage, nullptr);
        ::vkFreeMemory(m_Device->getRaw(), m_DepthImageMemory, nullptr);

        for (size_t i = 0; i < m_MaxFramesInFlight; i++)
        {
            ::vkDestroySemaphore(m_Device->getRaw(), m_ImageAvailableSemaphores[i], nullptr);
            ::vkDestroySemaphore(m_Device->getRaw(), m_RenderFinishedSemaphores[i], nullptr);

            ::vkDestroyFence(m_Device->getRaw(), m_InFlightFences[i], nullptr);
        }

        m_CommandPool = nullptr;

        m_Pipeline = nullptr;

        // Destroy old swapchain
        for (auto framebuffer : m_Framebuffers)
        {
            ::vkDestroyFramebuffer(m_Device->getRaw(), framebuffer, nullptr);
        }

        for (auto& image : m_Images)
        {
            ::vkDestroyImageView(m_Device->getRaw(), image.imageView, nullptr);
        }

        ::vkDestroySwapchainKHR(m_Device->getRaw(), m_Swapchain, nullptr);

        ::vkDestroyDescriptorPool(m_Device->getRaw(), m_DescriptorPool, nullptr);

        m_Shader = nullptr;

        ::vkDestroyRenderPass(m_Device->getRaw(), m_RenderPass, nullptr);

        VkInstance instance = VulkanContext::getInstance();
        ::vkDestroySurfaceKHR(instance, m_Surface, nullptr);
    }

    void VulkanSwapchain::resize(uint32_t width, uint32_t height)
    {
        m_Device->waitIdle();
        createSwapchain(width, height);
    }

    void VulkanSwapchain::drawFrame()
    {
        // Begin command buffer >>>
        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
            .pInheritanceInfo = nullptr
        };
        VK_CHECK(::vkBeginCommandBuffer(getCurrentCommandBuffer(), &beginInfo));
        // <<< Begin command buffer

        std::array<VkClearValue, 2> clearValues;
        clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0u };

        // Begin render pass >>>
        VkRenderPassBeginInfo renderPassInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = m_RenderPass,
            .framebuffer = getCurrentFramebuffer(),
            .renderArea = {
                .offset = { 0, 0 },
                .extent = m_SwapchainExtent
            },
            .clearValueCount = static_cast<uint32_t>(clearValues.size()),
            .pClearValues = clearValues.data()
        };
        ::vkCmdBeginRenderPass(getCurrentCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        // <<< Begin render pass

        // Bind pipeline
        VkPipeline graphicsPipeline = m_Pipeline->getRaw();
        ::vkCmdBindPipeline(getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(m_SwapchainExtent.width),
            .height = static_cast<float>(m_SwapchainExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(getCurrentCommandBuffer(), 0, 1, &viewport);

        VkRect2D scissor = {
            .offset = { 0, 0 },
            .extent = m_SwapchainExtent
        };
        vkCmdSetScissor(getCurrentCommandBuffer(), 0, 1, &scissor);

        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(getCurrentCommandBuffer(), 0, 1, &m_VertexBuffer, offsets);
        vkCmdBindIndexBuffer(getCurrentCommandBuffer(), m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->getLayout(), 0, 1, &m_DescriptorSets[m_CurrentFramebufferIndex], 0, nullptr);


        // --------------------- Draw call ---------------------
        vkCmdDrawIndexed(getCurrentCommandBuffer(), static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        // --------------------- Draw call ---------------------


        // End render pass
        ::vkCmdEndRenderPass(getCurrentCommandBuffer());

        // End command buffer
        VK_CHECK(::vkEndCommandBuffer(getCurrentCommandBuffer()));
    }

    void VulkanSwapchain::beginFrame()
    {
        ::vkWaitForFences(m_Device->getRaw(), 1, &m_InFlightFences[m_CurrentFramebufferIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
        ::vkResetFences(m_Device->getRaw(), 1, &m_InFlightFences[m_CurrentFramebufferIndex]);

        // Acquire next image >>>
        VkResult result = ::vkAcquireNextImageKHR(
            m_Device->getRaw(),
            m_Swapchain,
            std::numeric_limits<uint64_t>::max(),
            m_ImageAvailableSemaphores[m_CurrentFramebufferIndex],
            VK_NULL_HANDLE,
            &m_CurrentImageIndex
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            this->resize(m_SwapchainExtent.width, m_SwapchainExtent.height);
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            VK_CHECK(result);
        }
        // <<< Acquire next image

        static auto startTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTime).count();

        // [TODO] Move this to a separate function
        UniformBufferObject ubo{
            .model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            .proj = glm::perspective(glm::radians(45.0f), m_SwapchainExtent.width / (float)m_SwapchainExtent.height, 0.1f, 10.0f)
        };
        ubo.proj[1][1] *= -1;
        memcpy(m_MappedUniformBuffers[m_CurrentFramebufferIndex], &ubo, sizeof(ubo));


        vkResetCommandBuffer(getCurrentCommandBuffer(), 0);
    }

    void VulkanSwapchain::present()
    {
        std::vector<VkSemaphore> waitSemaphores = { m_ImageAvailableSemaphores[m_CurrentFramebufferIndex]};
        std::vector<VkSemaphore> signalSemaphores = { m_RenderFinishedSemaphores[m_CurrentFramebufferIndex]};
        std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
            .pWaitSemaphores = waitSemaphores.data(),
            .pWaitDstStageMask = waitStages.data(),
            .commandBufferCount = 1,
            .pCommandBuffers = &m_CommandBuffers[m_CurrentFramebufferIndex],
            .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
            .pSignalSemaphores = signalSemaphores.data()
        };

        VK_CHECK(::vkQueueSubmit(m_Device->getGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFramebufferIndex]));

        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
            .pWaitSemaphores = signalSemaphores.data(),
            .swapchainCount = 1,
            .pSwapchains = &m_Swapchain,
            .pImageIndices = &m_CurrentImageIndex,
            .pResults = nullptr
        };
        VkResult result = ::vkQueuePresentKHR(m_Device->getGraphicsQueue(), &presentInfo);
        VK_CHECK(result);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            this->resize(m_SwapchainExtent.width, m_SwapchainExtent.height);
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            VK_CHECK(result);
        }

        m_CurrentFramebufferIndex = (m_CurrentFramebufferIndex + 1) % m_MaxFramesInFlight;
    }

    void VulkanSwapchain::chooseSurfaceFormat()
    {
        auto physicalDevice = m_Device->getPhysicalDevice();

        uint32_t formatCount = 0;
        ::vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice->getRaw(), m_Surface, &formatCount, nullptr);
        AST_CORE_ASSERT(formatCount != 0, "Failed to get surface formats");

        std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
        ::vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice->getRaw(), m_Surface, &formatCount, surfaceFormats.data());

        for (const auto& surfaceFormat : surfaceFormats)
        {
            if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB
                && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                m_ImageFormat = surfaceFormat.format;
                m_ColorSpace = surfaceFormat.colorSpace;
                return;
            }
        }

        m_ImageFormat = surfaceFormats[0].format;
        m_ColorSpace = surfaceFormats[0].colorSpace;
    }

    void VulkanSwapchain::getQueueIndices()
    {
        auto physicalDevice = m_Device->getPhysicalDevice();

        std::optional<uint32_t> graphicsFamily = std::nullopt;
        std::optional<uint32_t> presentFamily = std::nullopt;
        auto& queueFamilyProperties = physicalDevice->getQueueFamilyProperties();
        for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
            if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            VK_CHECK(::vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice->getRaw(), i, m_Surface, &presentSupport));

            if (presentSupport) {
                presentFamily = i;
            }

            if (graphicsFamily.has_value() && presentFamily.has_value()) {
                break;
            }
        }
        AST_CORE_ASSERT(graphicsFamily.has_value(), "Failed to find graphics queue family");
        AST_CORE_ASSERT(presentFamily.has_value(), "Failed to find present queue family");

        m_GraphicsQueueIndex = graphicsFamily.value();
        m_PresentQueueIndex = presentFamily.value();
    }

    void VulkanSwapchain::getSwapchainImages()
    {
        // Get images >>>
        uint32_t imageCount = 0;
        ::vkGetSwapchainImagesKHR(m_Device->getRaw(), m_Swapchain, &imageCount, nullptr);
        AST_CORE_ASSERT(imageCount != 0, "Failed to get swapchain images");

        std::vector<VkImage> images(imageCount);
        ::vkGetSwapchainImagesKHR(m_Device->getRaw(), m_Swapchain, &imageCount, images.data());
        // <<< Get images

        // Create image views >>>
        m_Images.resize(imageCount);
        for (size_t i = 0; i < m_Images.size(); i++)
        {
            m_Images[i].image = images[i];
            m_Images[i].imageView = createImageView(
                images[i],
                m_ImageFormat,
                VK_IMAGE_ASPECT_COLOR_BIT
            );
        }
        // <<< Create image views
    }

    void VulkanSwapchain::createRenderPass()
    {
        VkAttachmentDescription colorAttachment{
            .flags = 0,
            .format = m_ImageFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        VkAttachmentReference colorAttachmentRef{
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkAttachmentDescription depthAttachment{
            .format = m_Device->getPhysicalDevice()->getDepthFormat(),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        VkAttachmentReference depthAttachmentRef{
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        VkSubpassDescription subpass{
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = &depthAttachmentRef,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr
        };

        VkSubpassDependency dependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        };

        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

        VkRenderPassCreateInfo renderPassInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency
        };
        VK_CHECK(::vkCreateRenderPass(m_Device->getRaw(), &renderPassInfo, nullptr, &m_RenderPass));
    }

    void VulkanSwapchain::createFramebuffers()
    {
        m_Framebuffers.resize(m_Images.size());

        VkFramebufferCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_RenderPass,
            .width = m_SwapchainExtent.width,
            .height = m_SwapchainExtent.height,
            .layers = 1
        };

        for (size_t i = 0; i < m_Framebuffers.size(); ++i)
        {
            std::array<VkImageView, 2> attachments = { m_Images[i].imageView, m_DepthImageView };

            createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            createInfo.pAttachments = attachments.data();

            VK_CHECK(::vkCreateFramebuffer(
                m_Device->getRaw(),
                &createInfo,
                nullptr,
                &m_Framebuffers[i]
            ));
        }
    }

    void VulkanSwapchain::createSyncObjects()
    {
        m_ImageAvailableSemaphores.resize(m_MaxFramesInFlight);
        m_RenderFinishedSemaphores.resize(m_MaxFramesInFlight);

        m_InFlightFences.resize(m_MaxFramesInFlight);

        VkSemaphoreCreateInfo semaphoreInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        VkFenceCreateInfo fenceInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        for (size_t i = 0; i < m_MaxFramesInFlight; i++)
        {
            VK_CHECK(::vkCreateSemaphore(m_Device->getRaw(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]));
            VK_CHECK(::vkCreateSemaphore(m_Device->getRaw(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]));

            VK_CHECK(::vkCreateFence(m_Device->getRaw(), &fenceInfo, nullptr, &m_InFlightFences[i]));
        }
    }

    void VulkanSwapchain::createBuffer(
        VkDeviceSize bufferSize,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory
    )
    {
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = bufferSize;
        bufferCreateInfo.usage = usage;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK(::vkCreateBuffer(m_Device->getRaw(), &bufferCreateInfo, nullptr, &buffer));

        VkMemoryRequirements memoryRequirements;
        ::vkGetBufferMemoryRequirements(m_Device->getRaw(), buffer, &memoryRequirements);

        auto& memoryProperties = m_Device->getPhysicalDevice()->getMemoryProperties();
        uint32_t memoryTypeIndex = 0;
        for (; memoryTypeIndex < memoryProperties.memoryTypeCount; memoryTypeIndex++)
        {
            if ((memoryRequirements.memoryTypeBits & BIT(memoryTypeIndex))
                && memoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & properties)
            {
                break;
            }
        }
        AST_CORE_ASSERT(memoryTypeIndex < memoryProperties.memoryTypeCount, "Failed to find suitable memory type");

        VkMemoryAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = memoryTypeIndex,
        };
        VK_CHECK(::vkAllocateMemory(m_Device->getRaw(), &allocateInfo, nullptr, &bufferMemory));
        VK_CHECK(::vkBindBufferMemory(m_Device->getRaw(), buffer, bufferMemory, 0));
    }

    void VulkanSwapchain::copyBuffer(
        VkBuffer srcBuffer,
        VkBuffer dstBuffer,
        VkDeviceSize size
    )
    {
        VkCommandBuffer commandBuffer = m_CommandPool->allocateCommandBuffer();
        beginOneTimeCommandBuffer(commandBuffer);

        VkBufferCopy copyRegion{
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size
        };
        ::vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endOneTimeCommandBuffer(commandBuffer);
    }

    void VulkanSwapchain::createDescriptorPool()
    {
        std::vector<VkDescriptorPoolSize> poolSizes{
            // Uniform buffer
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = static_cast<uint32_t>(m_MaxFramesInFlight)
            },
            // Sampler
            {
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = static_cast<uint32_t>(m_MaxFramesInFlight)
            }
        };

        VkDescriptorPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = static_cast<uint32_t>(m_MaxFramesInFlight),
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data()
        };

        VK_CHECK(::vkCreateDescriptorPool(m_Device->getRaw(), &poolInfo, nullptr, &m_DescriptorPool));
    }

    void VulkanSwapchain::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
    {
        VkImageCreateInfo imageInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = {
                .width = width,
                .height = height,
                .depth = 1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = tiling,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        VK_CHECK(::vkCreateImage(m_Device->getRaw(), &imageInfo, nullptr, &image));

        VkMemoryRequirements memoryRequirements;
        ::vkGetImageMemoryRequirements(m_Device->getRaw(), image, &memoryRequirements);

        auto& memoryProperties = m_Device->getPhysicalDevice()->getMemoryProperties();
        uint32_t memoryTypeIndex = 0;
        for (; memoryTypeIndex < memoryProperties.memoryTypeCount; memoryTypeIndex++)
        {
            if ((memoryRequirements.memoryTypeBits & BIT(memoryTypeIndex))
                && memoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & properties)
            {
                break;
            }
        }
        AST_CORE_ASSERT(memoryTypeIndex < memoryProperties.memoryTypeCount, "Failed to find suitable memory type");

        VkMemoryAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = memoryTypeIndex
        };
        VK_CHECK(::vkAllocateMemory(m_Device->getRaw(), &allocateInfo, nullptr, &imageMemory));

        VK_CHECK(::vkBindImageMemory(m_Device->getRaw(), image, imageMemory, 0));
    }

    VkImageView VulkanSwapchain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
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
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        VK_CHECK(::vkCreateImageView(m_Device->getRaw(), &viewInfo, nullptr, &imageView));
        return imageView;
    }

    void VulkanSwapchain::beginOneTimeCommandBuffer(VkCommandBuffer& commandBuffer)
    {
        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        VK_CHECK(::vkBeginCommandBuffer(commandBuffer, &beginInfo));
    }

    void VulkanSwapchain::endOneTimeCommandBuffer(VkCommandBuffer commandBuffer)
    {
        VK_CHECK(::vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
        };
        VK_CHECK(::vkQueueSubmit(m_Device->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

        ::vkQueueWaitIdle(m_Device->getGraphicsQueue());

        ::vkFreeCommandBuffers(m_Device->getRaw(), m_CommandPool->getGraphicsCommandPool(), 1, &commandBuffer);
    }

    void VulkanSwapchain::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = m_CommandPool->allocateCommandBuffer();
        beginOneTimeCommandBuffer(commandBuffer);

        VkImageMemoryBarrier barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            AST_CORE_ASSERT(false, "Unsupported layout transition");
        }

        ::vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        endOneTimeCommandBuffer(commandBuffer);
    }

    void VulkanSwapchain::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        VkCommandBuffer commandBuffer = m_CommandPool->allocateCommandBuffer();
        beginOneTimeCommandBuffer(commandBuffer);

        VkBufferImageCopy region{
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .imageOffset = { 0, 0, 0 },
            .imageExtent = { width, height, 1 }
        };

        ::vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        endOneTimeCommandBuffer(commandBuffer);
    }

    void VulkanSwapchain::loadModel(const std::string& modelPath)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        bool loaded = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str());
        AST_ASSERT(loaded, "Failed to load model.");

        std::unordered_map<Vertex, uint32_t> uniqueVertices;
        for (auto& shape : shapes)
        {
            for (auto& index : shape.mesh.indices)
            {
                Vertex vertex{
                    .position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    },
                    .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                    .texCoord = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    }
                };

                if (uniqueVertices.find(vertex) == uniqueVertices.end())
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }

        AST_INFO("Loaded model: {0}", modelPath);
        AST_INFO("Vertices: {0}, Indices: {1}", vertices.size(), indices.size());
    }
}
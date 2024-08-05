#include "pch.hpp"

#include "Astranox/platform/vulkan/VulkanSwapchain.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

#include "Astranox/core/Application.hpp"

#include <GLFW/glfw3.h>

namespace Astranox
{
    VulkanSwapchain::VulkanSwapchain(Ref<VulkanDevice> device)
        : m_Device(device)
    {
    }

    void VulkanSwapchain::createSurface()
    {
        VkInstance instance = VulkanContext::getInstance();

        Window& window = Application::get().getWindow();
        GLFWwindow* windowHandle = static_cast<GLFWwindow*>(window.getHandle());

        VkResult result = ::glfwCreateWindowSurface(instance, windowHandle, nullptr, &m_Surface);
        VK_CHECK(result);

        getQueueIndices();
        chooseSurfaceFormat();
    }

    void VulkanSwapchain::createSwapchain()
    {
        auto physicalDevice = m_Device->getPhysicalDevice();

        // Choose extent >>>
        VkSurfaceCapabilitiesKHR surfaceCapabilities{};
        VkResult result = ::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice->getRaw(), m_Surface, &surfaceCapabilities);
        VK_CHECK(result);

        if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            m_SwapchainExtent = surfaceCapabilities.currentExtent;
        } else {
            int width, height;

            // [TODO] This should not be platform specific, but for now, we call GLFW function.
            GLFWwindow* windowHandle = static_cast<GLFWwindow*>(Application::get().getWindow().getHandle());
            ::glfwGetFramebufferSize(windowHandle, &width, &height);

            m_SwapchainExtent.width = std::clamp(static_cast<uint32_t>(width), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
            m_SwapchainExtent.height = std::clamp(static_cast<uint32_t>(height), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
        }

        if (m_SwapchainExtent.width == 0 || m_SwapchainExtent.height == 0) {
            return;
        }
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

        VkSwapchainKHR oldSwapchain = m_Swapchain;

        VkSwapchainCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
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
            .oldSwapchain = oldSwapchain
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

        result = ::vkCreateSwapchainKHR(m_Device->getRaw(), &createInfo, nullptr, &m_Swapchain);
        VK_CHECK(result);

        getSwapchainImages();

        createRenderPass();
        createFramebuffers();
        createCommandBuffers();

        createSyncObjects();

        // Remove these

        std::string vertexShaderPath = "../Astranox-Rasterization/assets/shaders/vert.spv";
        std::string fragmentShaderPath = "../Astranox-Rasterization/assets/shaders/frag.spv";
        m_Shader = VulkanShader::create(vertexShaderPath, fragmentShaderPath);

        m_Pipeline = Ref<VulkanPipeline>::create(m_Shader);
        m_Pipeline->createPipeline();
    }

    void VulkanSwapchain::destroy()
    {
        m_Device->waitIdle();

        for (size_t i = 0; i < m_MaxFramesInFlight; i++)
        {
            ::vkDestroySemaphore(m_Device->getRaw(), m_ImageAvailableSemaphores[i], nullptr);
            ::vkDestroySemaphore(m_Device->getRaw(), m_RenderFinishedSemaphores[i], nullptr);

            ::vkDestroyFence(m_Device->getRaw(), m_InFlightFences[i], nullptr);
        }

        m_CommandPool = nullptr;

        m_Pipeline = nullptr;
        m_Shader = nullptr;

        for (auto framebuffer : m_Framebuffers)
        {
            ::vkDestroyFramebuffer(m_Device->getRaw(), framebuffer, nullptr);
        }
        m_Framebuffers.clear();

        ::vkDestroyRenderPass(m_Device->getRaw(), m_RenderPass, nullptr);

        for (auto& image : m_Images)
        {
            ::vkDestroyImageView(m_Device->getRaw(), image.imageView, nullptr);
        }

        ::vkDestroySwapchainKHR(m_Device->getRaw(), m_Swapchain, nullptr);

        VkInstance instance = VulkanContext::getInstance();
        ::vkDestroySurfaceKHR(instance, m_Surface, nullptr);
    }

    void VulkanSwapchain::drawFrame()
    {
        ::vkWaitForFences(m_Device->getRaw(), 1, &m_InFlightFences[m_CurrentFramebufferIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
        ::vkResetFences(m_Device->getRaw(), 1, &m_InFlightFences[m_CurrentFramebufferIndex]);

        acquireNextImage();
        vkResetCommandBuffer(getCurrentCommandBuffer(), 0);

        // Begin command buffer >>>
        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
            .pInheritanceInfo = nullptr
        };
        VkResult result = ::vkBeginCommandBuffer(getCurrentCommandBuffer(), &beginInfo);
        VK_CHECK(result);
        // <<< Begin command buffer

        VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };  // Black

        // Begin render pass >>>
        VkRenderPassBeginInfo renderPassInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = m_RenderPass,
            .framebuffer = getCurrentFramebuffer(),
            .renderArea = {
                .offset = { 0, 0 },
                .extent = { m_SwapchainExtent }
            },
            .clearValueCount = 1,
            .pClearValues = &clearColor
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

        // Draw call
        vkCmdDraw(getCurrentCommandBuffer(), 3, 1, 0, 0);

        // End render pass
        ::vkCmdEndRenderPass(getCurrentCommandBuffer());

        // End command buffer
        result = ::vkEndCommandBuffer(getCurrentCommandBuffer());
        VK_CHECK(result);


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

        result = ::vkQueueSubmit(m_Device->getGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFramebufferIndex]);
        VK_CHECK(result);

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
        result = ::vkQueuePresentKHR(m_Device->getGraphicsQueue(), &presentInfo);
        VK_CHECK(result);

        m_CurrentFramebufferIndex = (m_CurrentFramebufferIndex + 1) % m_MaxFramesInFlight;
    }

    void VulkanSwapchain::beginFrame() const
    {
        //m_CurrentImageIndex = acquireNextImage();
    }

    void VulkanSwapchain::present() const
    {
        //VkPresentInfoKHR presentInfo{
        //    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        //    .pNext = nullptr,
        //    .waitSemaphoreCount = 0,
        //    .pWaitSemaphores = nullptr,
        //    .swapchainCount = 1,
        //    .pSwapchains = &m_Swapchain,
        //    .pImageIndices = nullptr,
        //    .pResults = nullptr
        //};

        //VkResult result = ::vkQueuePresentKHR(m_Device->getGraphicsQueue(), &presentInfo);
        //VK_CHECK(result);
    }

    void VulkanSwapchain::acquireNextImage()
    {
        VkResult result = ::vkAcquireNextImageKHR(
            m_Device->getRaw(),
            m_Swapchain,
            std::numeric_limits<uint64_t>::max(),
            m_ImageAvailableSemaphores[m_CurrentFramebufferIndex],
            VK_NULL_HANDLE,
            &m_CurrentImageIndex
        );
        VK_CHECK(result);
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
            VkResult result = ::vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice->getRaw(), i, m_Surface, &presentSupport);
            VK_CHECK(result);

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

            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_ImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            VkResult result = ::vkCreateImageView(m_Device->getRaw(), &createInfo, nullptr, &m_Images[i].imageView);
            VK_CHECK(result);
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

        VkSubpassDescription subpass{
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr
        };

        VkSubpassDependency dependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        };

        VkRenderPassCreateInfo renderPassInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &colorAttachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency
        };

        VkResult result = ::vkCreateRenderPass(m_Device->getRaw(), &renderPassInfo, nullptr, &m_RenderPass);
        VK_CHECK(result);
    }

    void VulkanSwapchain::createFramebuffers()
    {
        m_Framebuffers.resize(m_Images.size());

        VkFramebufferCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = m_RenderPass,
            .attachmentCount = 1,
            .width = m_SwapchainExtent.width,
            .height = m_SwapchainExtent.height,
            .layers = 1
        };

        for (size_t i = 0; i < m_Framebuffers.size(); ++i)
        {
            createInfo.pAttachments = &m_Images[i].imageView;

            VkResult result = ::vkCreateFramebuffer(
                m_Device->getRaw(),
                &createInfo,
                nullptr,
                &m_Framebuffers[i]
            );
            VK_CHECK(result);
        }
    }

    void VulkanSwapchain::createCommandBuffers()
    {
        m_CommandPool = Ref<VulkanCommandPool>::create();

        m_CommandBuffers = m_CommandPool->allocateCommandBuffers(m_MaxFramesInFlight);
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
            VkResult result = ::vkCreateSemaphore(m_Device->getRaw(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]);
            VK_CHECK(result);
            result = ::vkCreateSemaphore(m_Device->getRaw(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]);
            VK_CHECK(result);

            result = ::vkCreateFence(m_Device->getRaw(), &fenceInfo, nullptr, &m_InFlightFences[i]);
            VK_CHECK(result);
        }
    }
}
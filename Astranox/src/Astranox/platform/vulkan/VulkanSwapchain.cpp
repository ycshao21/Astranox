#include "pch.hpp"
#include <filesystem>

#include "Astranox/platform/vulkan/VulkanSwapchain.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanBufferManager.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

#include "Astranox/platform/vulkan/VulkanUniformBuffer.hpp"

#include "Astranox/core/Application.hpp"

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

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
        AST_CORE_DEBUG("Desired number of swapchain images: {0}", desiredImageCount);

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

        if (m_Swapchain) {
            ::vkDestroySwapchainKHR(m_Device->getRaw(), m_Swapchain, nullptr);
        }
        VK_CHECK(::vkCreateSwapchainKHR(m_Device->getRaw(), &createInfo, nullptr, &m_Swapchain));

        // Destroy old image views
        for (auto& image : m_Images)
        {
            ::vkDestroyImageView(m_Device->getRaw(), image.imageView, nullptr);
        }
        getSwapchainImages();

        VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

        //if (m_ColorAttachment.image)
        //{
        //    ::vkDestroyImageView(m_Device->getRaw(), m_ColorAttachment.imageView, nullptr);
        //    ::vkDestroyImage(m_Device->getRaw(), m_ColorAttachment.image, nullptr);
        //    ::vkFreeMemory(m_Device->getRaw(), m_ColorAttachment.memory, nullptr);
        //}
        //VkFormat colorFormat = m_ImageFormat;

        //VulkanBufferManager::createImage(
        //    m_SwapchainExtent.width,
        //    m_SwapchainExtent.height,
        //    1,
        //    msaaSamples,
        //    colorFormat,
        //    VK_IMAGE_TILING_OPTIMAL,
        //    VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        //    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        //    m_ColorAttachment.image,
        //    m_ColorAttachment.memory
        //);

        //m_ColorAttachment.imageView = createImageView(
        //    m_ColorAttachment.image,
        //    colorFormat,
        //    VK_IMAGE_ASPECT_COLOR_BIT,
        //    1
        //);

        if (m_DepthStencil.image)
        {
            ::vkDestroyImageView(m_Device->getRaw(), m_DepthStencil.imageView, nullptr);
            ::vkDestroyImage(m_Device->getRaw(), m_DepthStencil.image, nullptr);
            ::vkFreeMemory(m_Device->getRaw(), m_DepthStencil.memory, nullptr);
        }
        uint32_t depthMipLevels = 1;
        VulkanBufferManager::createImage(
            m_SwapchainExtent.width,
            m_SwapchainExtent.height,
            depthMipLevels,
            msaaSamples,
            physicalDevice->getDepthFormat(),
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_DepthStencil.image,
            m_DepthStencil.memory
        );

        m_DepthStencil.imageView = createImageView(
            m_DepthStencil.image,
            physicalDevice->getDepthFormat(),
            VK_IMAGE_ASPECT_DEPTH_BIT,
            depthMipLevels
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

        uint32_t framesInFlight = m_Images.size();
        m_CommandBuffers = m_Device->getCommandPool()->allocateCommandBuffers(framesInFlight);

        if (m_InFlightFences.empty())
        {
            createSyncObjects();
        }
    }

    void VulkanSwapchain::destroy()
    {
        m_Device->waitIdle();

        VkDevice device = m_Device->getRaw();

        ::vkDestroyImageView(device, m_DepthStencil.imageView, nullptr);
        ::vkDestroyImage(device, m_DepthStencil.image, nullptr);
        ::vkFreeMemory(device, m_DepthStencil.memory, nullptr);

        //::vkDestroyImageView(device, m_ColorAttachment.imageView, nullptr);
        //::vkDestroyImage(device, m_ColorAttachment.image, nullptr);
        //::vkFreeMemory(device, m_ColorAttachment.memory, nullptr);

        uint32_t framesInFlight = m_Images.size();
        for (size_t i = 0; i < framesInFlight; i++)
        {
            ::vkDestroySemaphore(device, m_ImageAvailableSemaphores[i], nullptr);
            ::vkDestroySemaphore(device, m_RenderFinishedSemaphores[i], nullptr);

            ::vkDestroyFence(device, m_InFlightFences[i], nullptr);
        }

        for (auto framebuffer : m_Framebuffers)
        {
            ::vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto& image : m_Images)
        {
            ::vkDestroyImageView(device, image.imageView, nullptr);
        }

        ::vkDestroySwapchainKHR(device, m_Swapchain, nullptr);

        ::vkDestroyRenderPass(device, m_RenderPass, nullptr);

        VkInstance instance = VulkanContext::getInstance();
        ::vkDestroySurfaceKHR(instance, m_Surface, nullptr);
    }

    void VulkanSwapchain::resize(uint32_t width, uint32_t height)
    {
        m_Device->waitIdle();
        createSwapchain(width, height);
    }

    void VulkanSwapchain::beginFrame()
    {
        ::vkWaitForFences(m_Device->getRaw(), 1, &m_InFlightFences[m_CurrentFrameIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
        ::vkResetFences(m_Device->getRaw(), 1, &m_InFlightFences[m_CurrentFrameIndex]);

        VkResult result = ::vkAcquireNextImageKHR(
            m_Device->getRaw(),
            m_Swapchain,
            std::numeric_limits<uint64_t>::max(),
            m_ImageAvailableSemaphores[m_CurrentFrameIndex],
            VK_NULL_HANDLE,
            &m_CurrentImageIndex
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            this->resize(m_SwapchainExtent.width, m_SwapchainExtent.height);
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            VK_CHECK(result);
        }

        vkResetCommandBuffer(getCurrentCommandBuffer(), 0);
    }

    void VulkanSwapchain::present()
    {
        std::vector<VkSemaphore> waitSemaphores = { m_ImageAvailableSemaphores[m_CurrentFrameIndex]};
        std::vector<VkSemaphore> signalSemaphores = { m_RenderFinishedSemaphores[m_CurrentFrameIndex]};
        std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
            .pWaitSemaphores = waitSemaphores.data(),
            .pWaitDstStageMask = waitStages.data(),
            .commandBufferCount = 1,
            .pCommandBuffers = &m_CommandBuffers[m_CurrentFrameIndex],
            .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
            .pSignalSemaphores = signalSemaphores.data()
        };

        VK_CHECK(::vkQueueSubmit(m_Device->getGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrameIndex]));

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

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            this->resize(m_SwapchainExtent.width, m_SwapchainExtent.height);
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            VK_CHECK(result);
        }

        uint32_t framesInFlight = m_Images.size();
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % framesInFlight;
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
        // Swapchain images >>>
        uint32_t imageCount = 0;
        ::vkGetSwapchainImagesKHR(m_Device->getRaw(), m_Swapchain, &imageCount, nullptr);
        AST_CORE_ASSERT(imageCount != 0, "Failed to get swapchain images");
        AST_CORE_INFO("Actual number of swapchain images: {0}", imageCount);

        std::vector<VkImage> images(imageCount);
        ::vkGetSwapchainImagesKHR(m_Device->getRaw(), m_Swapchain, &imageCount, images.data());
        // <<< Swapchain images

        // Swapchain image views >>>
        uint32_t swapchainMipLevels = 1;

        m_Images.resize(imageCount);
        for (size_t i = 0; i < m_Images.size(); i++)
        {
            m_Images[i].image = images[i];
            m_Images[i].imageView = createImageView(
                images[i],
                m_ImageFormat,
                VK_IMAGE_ASPECT_COLOR_BIT,
                swapchainMipLevels
            );
        }
        // <<< Swapchain image views
    }

    void VulkanSwapchain::createRenderPass()
    {
        VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

        VkAttachmentDescription colorAttachment{
            .flags = 0,
            .format = m_ImageFormat,
            .samples = msaaSamples,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            //.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL  // Use this if you want multisampling
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        VkAttachmentReference colorAttachmentRef{
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkAttachmentDescription depthAttachment{
            .format = m_Device->getPhysicalDevice()->getDepthFormat(),
            .samples = msaaSamples,
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

        //VkAttachmentDescription colorAttachmentResolve{
        //    .format = m_ImageFormat,
        //    .samples = VK_SAMPLE_COUNT_1_BIT,
        //    .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        //    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        //    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        //    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        //    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        //    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        //};

        //VkAttachmentReference colorAttachmentResolveRef{
        //    .attachment = 2,
        //    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        //};

        VkSubpassDescription subpass{
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = &depthAttachmentRef,
        };

        VkSubpassDependency dependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            //.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        };

        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        //std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };

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
            //std::array<VkImageView, 3> attachments = { m_ColorAttachment.imageView, m_DepthStencil.imageView, m_Images[i].imageView };
            std::array<VkImageView, 2> attachments = { m_Images[i].imageView, m_DepthStencil.imageView };

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
        uint32_t framesInFlight = m_Images.size();

        m_ImageAvailableSemaphores.resize(framesInFlight);
        m_RenderFinishedSemaphores.resize(framesInFlight);

        m_InFlightFences.resize(framesInFlight);

        VkSemaphoreCreateInfo semaphoreInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        VkFenceCreateInfo fenceInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        for (size_t i = 0; i < framesInFlight; i++)
        {
            VK_CHECK(::vkCreateSemaphore(m_Device->getRaw(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]));
            VK_CHECK(::vkCreateSemaphore(m_Device->getRaw(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]));

            VK_CHECK(::vkCreateFence(m_Device->getRaw(), &fenceInfo, nullptr, &m_InFlightFences[i]));
        }
    }

    VkImageView VulkanSwapchain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
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
        VK_CHECK(::vkCreateImageView(m_Device->getRaw(), &viewInfo, nullptr, &imageView));
        return imageView;
    }
}
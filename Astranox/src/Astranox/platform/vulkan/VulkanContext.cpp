#include "pch.hpp"

#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanPhysicalDevice.hpp"
#include "Astranox/platform/vulkan/VulkanDevice.hpp"
#include "Astranox/platform/vulkan/VulkanSwapchain.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

#include "Astranox/core/Application.hpp"

#include <GLFW/glfw3.h>

namespace Astranox
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    ) {
        AST_CORE_ERROR("Vulkan validation layer: {0}", pCallbackData->pMessage);
        return VK_FALSE;
    }

    void VulkanContext::init()
    {
        s_Context = this;
        AST_CORE_INFO("Creating Vulkan context...");

        createInstance();

        if (VK_ENABLE_VALIDATION_LAYERS)
        {
            setupDebugMessenger();
        }

        m_PhysicalDevice = VulkanPhysicalDevice::pick();
        m_Device = Ref<VulkanDevice>::create(m_PhysicalDevice);

        m_Swapchain = Ref<VulkanSwapchain>::create(m_Device);
        m_Swapchain->createSurface();
        m_Swapchain->createSwapchain();
    }

    void VulkanContext::destroy()
    {
        m_Swapchain->destroy();
        m_Swapchain = nullptr;

        m_Device->destroy();
        m_Device = nullptr;

        if (VK_ENABLE_VALIDATION_LAYERS)
        {
            // Destroy the debug messenger
            auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(s_Instance, "vkDestroyDebugUtilsMessengerEXT");
            AST_CORE_ASSERT(vkDestroyDebugUtilsMessengerEXT, "Failed to load vkDestroyDebugUtilsMessengerEXT");
            vkDestroyDebugUtilsMessengerEXT(s_Instance, m_DebugMessenger, nullptr);
        }

        ::vkDestroyInstance(s_Instance, nullptr);
        s_Instance = nullptr;

        s_Context = nullptr;
    }

    void VulkanContext::swapBuffers()
    {
    }

    void VulkanContext::createInstance()
    {
        VkInstanceCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
        };

        // --------------------- Application Info ---------------------
        VkApplicationInfo appInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "Astranox",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "Astranox",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_3
        };
        createInfo.pApplicationInfo = &appInfo;


        // --------------------- Validation Layers ---------------------
        // [NOTE] `validationLayers.data()` should stay valid until `vkCreateInstance` is called.
        // If `validationLayers` is defined in the "if" block, it will be destroyed after the block.
        // So, we define it here just to keep it valid.

        if (VK_ENABLE_VALIDATION_LAYERS) {
            AST_CORE_DEBUG("Validation layers are enabled.");
            VulkanUtils::checkValidationLayerSupport();
            createInfo.enabledLayerCount = static_cast<uint32_t>(VulkanUtils::validationLayers.size());
            createInfo.ppEnabledLayerNames = VulkanUtils::validationLayers.data();
        }


        // --------------------- Extensions  ---------------------
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = ::glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (VK_ENABLE_VALIDATION_LAYERS) {
            requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        VulkanUtils::checkInstanceExtensionSupport(requiredExtensions);
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();


        VkResult result = ::vkCreateInstance(&createInfo, nullptr, &s_Instance);
        VK_CHECK(result);
    }

    void VulkanContext::setupDebugMessenger()
    {
        AST_CORE_DEBUG("Setting up Vulkan debug messenger...");

        // Get function pointer
        auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)::vkGetInstanceProcAddr(s_Instance, "vkCreateDebugUtilsMessengerEXT");
        AST_CORE_ASSERT(vkCreateDebugUtilsMessengerEXT, "Failed to load vkCreateDebugUtilsMessengerEXT");


        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = 0,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debugCallback,
            .pUserData = nullptr
        };
        VkResult result = vkCreateDebugUtilsMessengerEXT(s_Instance, &debugCreateInfo, nullptr, &m_DebugMessenger);
        VK_CHECK(result);
    }
}

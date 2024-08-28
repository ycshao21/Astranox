#include "pch.hpp"

#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanPhysicalDevice.hpp"
#include "Astranox/platform/vulkan/VulkanDevice.hpp"
#include "Astranox/platform/vulkan/VulkanMemoryAllocator.hpp"
#include "Astranox/platform/vulkan/VulkanSwapchain.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

#include "Astranox/core/Application.hpp"

#include <GLFW/glfw3.h>

namespace Astranox
{
    /**
     * @brief Callback function for the Vulkan validation layer.
     */
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    ) {
        AST_CORE_ERROR("Vulkan validation layer: {0}", pCallbackData->pMessage);
        return VK_FALSE;
    }

    /////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Check if the required validation layers are available.
     */
    static void checkValidationLayerSupport()
    {
        uint32_t supportedLayerCount = 0;
        ::vkEnumerateInstanceLayerProperties(&supportedLayerCount, nullptr);

        std::vector<VkLayerProperties> supportedLayers(supportedLayerCount);
        ::vkEnumerateInstanceLayerProperties(&supportedLayerCount, supportedLayers.data());

        AST_CORE_DEBUG("Found {0} supported validation layers.", supportedLayerCount);
        //for (size_t i = 0; i < supportedLayerCount; i++) {
        //    AST_CORE_DEBUG("  {0}: {1}", i + 1, supportedLayers[i].layerName);
        //}
        AST_CORE_DEBUG("Required {0} validation layers:", VulkanUtils::validationLayers.size());
        for (size_t i = 0; i < VulkanUtils::validationLayers.size(); ++i) {
            AST_CORE_DEBUG("  {0}: {1}", i + 1, VulkanUtils::validationLayers[i]);
        }

        for (const char* layerName : VulkanUtils::validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : supportedLayers) {
                if (std::strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                AST_CORE_ASSERT(false, "Validation layer {0} requested, but not available!", layerName);
                return;
            }
        }

        AST_CORE_DEBUG("All required validation layers are available.");
    }

    static void checkInstanceExtensionSupport(const std::vector<const char*>& requiredExtensions)
    {
        uint32_t extensionCount = 0;
        ::vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        ::vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

        AST_CORE_DEBUG("Found {0} available instance extensions.", extensionCount);
        //for (size_t i = 0; i < extensionCount; i++) {
        //    AST_CORE_DEBUG("  {0}: {1}", i + 1, availableExtensions[i].extensionName);
        //}
        AST_CORE_DEBUG("Required {0} instance extensions:", requiredExtensions.size());
        for (size_t i = 0; i < requiredExtensions.size(); i++) {
            AST_CORE_DEBUG("  {0}: {1}", i + 1, requiredExtensions[i]);
        }

        for (const char* extensionName : requiredExtensions) {
            bool extensionFound = false;

            for (const auto& extension : availableExtensions) {
                if (std::strcmp(extensionName, extension.extensionName) == 0) {
                    extensionFound = true;
                    break;
                }
            }

            if (!extensionFound) {
                AST_CORE_ASSERT(false, "Extension {0} requested, but not available!", extensionName);
                return;
            }
        }

        AST_CORE_DEBUG("All required extensions are available.");
    }

    /////////////////////////////////////////////////////////////////////////////////

    static Ref<VulkanContext> s_ContextInstance = nullptr;

    void VulkanContext::init(uint32_t& width, uint32_t& height)
    {
        s_ContextInstance = this;
        AST_CORE_INFO("Creating Vulkan context...");

        createInstance();

        if (VK_ENABLE_VALIDATION_LAYERS)
        {
            setupDebugMessenger();
        }

        m_PhysicalDevice = VulkanPhysicalDevice::pick();
        m_Device = Ref<VulkanDevice>::create(m_PhysicalDevice);

        VulkanMemoryAllocator::init(m_Device);

        m_Swapchain = Ref<VulkanSwapchain>::create(m_Device);
        m_Swapchain->createSurface();
        m_Swapchain->createSwapchain(width, height);
    }

    void VulkanContext::destroy()
    {
        m_Swapchain->destroy();
        m_Swapchain = nullptr;

        VulkanMemoryAllocator::shutdown();

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

        s_ContextInstance = nullptr;
    }

    void VulkanContext::swapBuffers()
    {
        m_Swapchain->present();
    }

    Ref<VulkanContext> VulkanContext::get()
    {
        return s_ContextInstance;
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
            checkValidationLayerSupport();
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
        checkInstanceExtensionSupport(requiredExtensions);
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        VK_CHECK(::vkCreateInstance(&createInfo, nullptr, &s_Instance));
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
        VK_CHECK(vkCreateDebugUtilsMessengerEXT(s_Instance, &debugCreateInfo, nullptr, &m_DebugMessenger));
    }
}

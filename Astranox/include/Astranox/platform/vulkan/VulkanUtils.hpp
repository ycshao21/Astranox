#pragma once
#include "Astranox/core/Base.hpp"
#include "Astranox/platform/vulkan/VulkanPhysicalDevice.hpp"

#ifdef AST_ENABLE_ASSERTS
    #define VK_CHECK(x) Astranox::VulkanUtils::checkResult(x)
#else
    #define VK_CHECK(x)
#endif

#ifdef AST_CONFIG_DEBUG
    #define VK_ENABLE_VALIDATION_LAYERS 1
#else
    #define VK_ENABLE_VALIDATION_LAYERS 0
#endif

namespace Astranox
{
    namespace VulkanUtils
    {
        constexpr std::array<const char*, 1> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        constexpr std::array<const char*, 1> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        void checkValidationLayerSupport();
        void checkInstanceExtensionSupport(const std::vector<const char*>& requiredExtensions);
        void checkDeviceExtensionSupport(Ref<VulkanPhysicalDevice> physicalDevice);

        std::string vkResultToString(VkResult result);
        void checkResult(VkResult result);

        std::string vkPhysicalDeviceTypeToString(VkPhysicalDeviceType type);
    }
}

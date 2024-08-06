#pragma once
#include "Astranox/core/Base.hpp"
#include "VulkanPhysicalDevice.hpp"

#define VK_CHECK(result) Astranox::VulkanUtils::checkResult(result)

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

        inline void checkResult(VkResult result)
        {
            AST_CORE_ASSERT(result == VK_SUCCESS, "Vulkan error: {0}", vkResultToString(result));
        }


        std::string vkPhysicalDeviceTypeToString(VkPhysicalDeviceType type);
    }
}

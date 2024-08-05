#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

namespace Astranox
{
    namespace VulkanUtils
    {
        void checkValidationLayerSupport()
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
            for (size_t i = 0; i < VulkanUtils::validationLayers.size(); i++) {
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

        void checkInstanceExtensionSupport(const std::vector<const char*>& requiredExtensions)
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

        void checkDeviceExtensionSupport(Ref<VulkanPhysicalDevice> physicalDevice)
        {
            for (const char* extensionName : VulkanUtils::deviceExtensions)
            {
                AST_CORE_ASSERT(
                    physicalDevice->isExtentionSupported(extensionName),
                    "Physical device does not support {0}!", extensionName
                );
            }
        }

        std::string vkResultToString(VkResult result)
        {
            switch (result)
            {
                case VK_SUCCESS: return "VK_SUCCESS";
                case VK_NOT_READY: return "VK_NOT_READY";
                case VK_TIMEOUT: return "VK_TIMEOUT";
                case VK_EVENT_SET: return "VK_EVENT_SET";
                case VK_EVENT_RESET: return "VK_EVENT_RESET";
                case VK_INCOMPLETE: return "VK_INCOMPLETE";
                case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
                case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
                case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
                case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
                case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
                case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
                case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
                case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
                case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
                case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
                case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
                case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
                case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
                case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
                case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
                case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
                case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
                case VK_RESULT_MAX_ENUM: return "VK_RESULT_MAX_ENUM";
            }
            return "UNKNOWN";
        }

        void checkResult(VkResult result)
        {
            AST_CORE_ASSERT(result == VK_SUCCESS, std::format("Vulkan error: {0}", vkResultToString(result)));
        }

        std::string vkPhysicalDeviceTypeToString(VkPhysicalDeviceType type)
        {
            if (type == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            {
                return "Integrated GPU";
            } else if (type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                return "Discrete GPU";
            } else if (type == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
            {
                return "Virtual GPU";
            } else if (type == VK_PHYSICAL_DEVICE_TYPE_CPU)
            {
                return "CPU";
            } else
            {
                return "Other";
            }
        }
    }
}
#pragma once
#include "Astranox/core/Base.hpp"
#include "Astranox/rendering/VertexBufferLayout.hpp"
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



        constexpr VkFormat shaderDataTypeToVkFormat(ShaderDataType type)
        {
            switch (type)
            {
                case ShaderDataType::Float: { return VK_FORMAT_R32_SFLOAT; }
                case ShaderDataType::Vec2:  { return VK_FORMAT_R32G32_SFLOAT; }
                case ShaderDataType::Vec3:  { return VK_FORMAT_R32G32B32_SFLOAT; }
                case ShaderDataType::Vec4:  { return VK_FORMAT_R32G32B32A32_SFLOAT; }
                case ShaderDataType::Mat3:  { return VK_FORMAT_R32G32B32_SFLOAT; }
                case ShaderDataType::Mat4:  { return VK_FORMAT_R32G32B32A32_SFLOAT; }
                case ShaderDataType::Int:   { return VK_FORMAT_R32_SINT; }
                case ShaderDataType::Ivec2: { return VK_FORMAT_R32G32_SINT; }
                case ShaderDataType::Ivec3: { return VK_FORMAT_R32G32B32_SINT; }
                case ShaderDataType::Ivec4: { return VK_FORMAT_R32G32B32A32_SINT; }
                case ShaderDataType::Bool:  { return VK_FORMAT_R8_UINT; }
            }

            AST_CORE_ASSERT(false, "Unknown shader data type!");
            return VK_FORMAT_UNDEFINED;
        }
    }
}

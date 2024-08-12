#pragma once
#include "RendererAPI.hpp"

#include "Astranox/platform/vulkan/VulkanContext.hpp"

namespace Astranox
{
    struct RendererConfig
    {
        uint32_t framesInFlight = 2;
    };

    class Renderer
    {
    public:
		static void init();
        static void shutdown();

        static const RendererConfig& getConfig();

    private:
        //static VkSampleCountFlagBits getMaxUsableSampleCount();

    private:
        inline static Ref<RendererAPI> s_RendererAPI = nullptr;
    };
}

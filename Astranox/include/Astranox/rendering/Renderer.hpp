#pragma once
#include "RendererAPI.hpp"

namespace Astranox
{
    class Renderer
    {
    public:
		static void init();
        static void shutdown();

    private:
        inline static Ref<RendererAPI> s_RendererAPI = nullptr;
    };
}

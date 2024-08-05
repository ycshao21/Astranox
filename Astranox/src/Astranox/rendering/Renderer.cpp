#include "pch.hpp"
#include "Astranox/rendering/Renderer.hpp"

namespace Astranox
{
    void Renderer::init()
    {
        // Load shaders

        // Load textures

        // Initialize renderer api
        s_RendererAPI = Ref<RendererAPI>::create();
    }

    void Renderer::shutdown()
    {
        s_RendererAPI = nullptr;
    }
}


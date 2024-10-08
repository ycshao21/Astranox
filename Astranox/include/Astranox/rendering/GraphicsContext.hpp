#pragma once
#include "Astranox/core/RefCounted.hpp"

namespace Astranox
{
    class GraphicsContext: public RefCounted
    {
    public:
        static Ref<GraphicsContext> create();
        virtual ~GraphicsContext() = default;

        virtual void init(uint32_t& width, uint32_t& height) = 0;
        virtual void destroy() = 0;

    public:
        virtual void swapBuffers() = 0;
    };
}

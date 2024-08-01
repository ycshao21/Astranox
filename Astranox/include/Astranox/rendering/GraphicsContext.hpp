#pragma once
#include "Astranox/core/RefCounted.hpp"

namespace Astranox
{
    class GraphicsContext: public RefCounted
    {
    public:
        static Ref<GraphicsContext> create();
        virtual ~GraphicsContext() = default;

        virtual void init() = 0;
        virtual void destroy() = 0;

    public:
        virtual void swapBuffers() = 0;
    };
}

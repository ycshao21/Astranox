#pragma once

#include "Astranox/core/RefCounted.hpp"

namespace Astranox
{
    class VertexBuffer: public RefCounted
    {
    public:
        static Ref<VertexBuffer> create(uint32_t bytes);
        static Ref<VertexBuffer> create(void* data, uint32_t bytes);
        virtual ~VertexBuffer() = default;
        
        virtual void setData(const void* data, uint32_t bytes) = 0;
    };
}


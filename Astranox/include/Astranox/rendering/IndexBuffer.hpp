#pragma once

#include "Astranox/core/RefCounted.hpp"

namespace Astranox
{
    class IndexBuffer: public RefCounted
    {
    public:
        static Ref<IndexBuffer> create(uint32_t* data, uint32_t bytes);
        virtual ~IndexBuffer() = default;

    public:
        virtual uint32_t getCount() const = 0;
    };
}

#pragma once

#include "Astranox/core/RefCounted.hpp"

namespace Astranox
{
    class UniformBuffer: public RefCounted
    {
    public:
        static Ref<UniformBuffer> create(uint32_t bytes);
        virtual ~UniformBuffer() = default;

        virtual void setData(const void* data, uint32_t bytes, uint32_t offset = 0) = 0;
    };
}


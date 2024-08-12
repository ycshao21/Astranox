#pragma once

#include "Astranox/core/RefCounted.hpp"
#include "UniformBuffer.hpp"

namespace Astranox
{
    class UniformBufferArray: public RefCounted
    {
    public:
        static Ref<UniformBufferArray> create(uint32_t bytes);
        virtual ~UniformBufferArray() = default;

        virtual Ref<UniformBuffer> getBuffer(uint32_t frameIndex) = 0;
        virtual Ref<UniformBuffer> getCurrentBuffer() = 0;
    };
}

#pragma once

namespace Astranox
{
    struct Buffer
    {
        void* data = nullptr;
        uint32_t size = 0;

        Buffer() = default;

        Buffer(const void* data, uint32_t size)
            : data((void*)data), size(size)
        {
        }
    };
}

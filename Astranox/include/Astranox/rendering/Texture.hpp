#pragma once
#include "Astranox/core/RefCounted.hpp"

namespace Astranox
{
    class Texture: public RefCounted
    {
    public:
        static Ref<Texture> create(const std::filesystem::path& path, bool enableMipMaps);
        virtual ~Texture() = default;

        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;
        virtual uint32_t getChannels() const = 0;
        virtual uint32_t getMipLevels() const = 0;

        virtual const std::filesystem::path& getPath() const = 0;
    };
}

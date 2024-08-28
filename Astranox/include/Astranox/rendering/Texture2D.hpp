#pragma once
#include "Astranox/core/RefCounted.hpp"

namespace Astranox
{
    class Texture2D: public RefCounted
    {
    public:
        static Ref<Texture2D> create(const std::filesystem::path& path, bool enableMipMaps = false);
        static Ref<Texture2D> create(uint32_t width = 1, uint32_t height = 1, Buffer buffer = {});
        virtual ~Texture2D() = default;

        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;
        virtual uint32_t getChannels() const = 0;
        virtual uint32_t getMipLevels() const = 0;

        virtual const std::filesystem::path& getPath() const = 0;

    public:
        virtual bool operator==(const Texture2D& other) const = 0;
    };
}

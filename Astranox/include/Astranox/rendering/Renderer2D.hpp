#pragma once
#include <glm/glm.hpp>

#include "PerspectiveCamera.hpp"
#include "Texture2D.hpp"
#include "Astranox/core/RefCounted.hpp"

namespace Astranox
{
    class Renderer2D: public RefCounted
    {
    public:
        void init();
        void shutdown();

        void beginScene(const PerspectiveCamera& camera);
        void endScene();

        void drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
        void drawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
        void drawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = { 1.0f, 1.0f, 1.0f, 1.0f });
        void drawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = { 1.0f, 1.0f, 1.0f, 1.0f });

        void drawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float degrees, const glm::vec4& color);
        void drawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float degrees, const glm::vec4& color);

        struct Statistics
        {
            uint32_t drawCalls = 0;
            uint32_t quadCount = 0;

            uint32_t getTotalVertexCount() const { return quadCount * 4; };
            uint32_t getTotalIndexCount() const { return quadCount * 6; };
        };

        Statistics getStats() const;
        void resetStats();
    };
}

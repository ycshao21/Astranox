#pragma once
#include <glm/glm.hpp>

#include "PerspectiveCamera.hpp"
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
    };
}

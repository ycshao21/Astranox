#pragma once

namespace Astranox
{
    class Renderer2D
    {
    public:
        void init();
        void shutdown();

        void beginScene();
        void endScene();

        void drawQuad();
    };
}

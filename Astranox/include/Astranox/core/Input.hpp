#pragma once

#include "Keycodes.hpp"

namespace Astranox
{
    class Input
    {
    public:
        static bool isMouseButtonPressed(MouseButton button);
        static glm::vec2 GetMousePosition();
        static void setCursorMode(CursorMode mode);

        static bool isKeyPressed(Key keycode);
    };
}

#include "pch.hpp"
#include "Astranox/core/Application.hpp"
#include "Astranox/core/Input.hpp"

namespace Astranox
{
    bool Input::isMouseButtonPressed(MouseButton button)
    {
        Window& window = Application::get().getWindow();
        MouseButtonState state = window.getMouseButtonState(button);
        return state == MouseButtonState::Pressed;
    }

    glm::vec2 Input::GetMousePosition()
    {
        Window& window = Application::get().getWindow();
        glm::vec2 pos = window.getCursorPosition();
        return pos;
    }

    void Input::setCursorMode(CursorMode mode)
    {
        Window& window = Application::get().getWindow();
        window.setCursorMode(mode);
    }

    bool Input::isKeyPressed(Key key)
    {
        Window& window = Application::get().getWindow();
        KeyState state = window.getKeyState(key);

        return state == KeyState::Pressed || state == KeyState::Repeat;
    }
}
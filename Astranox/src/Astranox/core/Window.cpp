#include "pch.hpp"

#include "Astranox/core/Window.hpp"
#include "Astranox/platform/windows/WindowsWindow.hpp"

namespace Astranox
{
    std::unique_ptr<Window> Window::create(const WindowSpecification& spec)
    {
#ifdef AST_PLATFORM_WINDOWS
        return std::make_unique<WindowsWindow>(spec);
#else
        AST_CORE_ASSERT(false, "Unknown platform!");
        return nullptr;
#endif
    }
}

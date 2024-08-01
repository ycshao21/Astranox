#include "pch.hpp"

#include "Astranox/core/Window.hpp"
#include "Astranox/platform/windows/WindowsWindow.hpp"

namespace Astranox
{
    std::unique_ptr<Window> Window::create(const WindowSpecification& spec)
    {
#ifdef AW_PLATFORM_WINDOWS
        return std::make_unique<WindowsWindow>(spec);
#else
        AW_CORE_ASSERT(false, "Unknown platform!");
        return nullptr;
#endif
    }
}

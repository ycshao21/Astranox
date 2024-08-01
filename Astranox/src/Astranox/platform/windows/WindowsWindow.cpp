#include "pch.hpp"
#include "Astranox/platform/windows/WindowsWindow.hpp"

#include <GLFW/glfw3.h>

namespace Astranox
{
    static bool s_GLFWInitialized = false;

    WindowsWindow::WindowsWindow(const WindowSpecification& spec)
    {
        m_Data.title = spec.title;
        m_Data.width = static_cast<int>(spec.width);
        m_Data.height = static_cast<int>(spec.height);
        m_Data.vsync = spec.vsync;
    }

    void WindowsWindow::init()
    {
        if (!s_GLFWInitialized)
        {
            // Initialize GLFW
            // [NOTE] glfwInit() cannot be called directly in AW_CORE_ASSERT,
            // because the window may not be created in Release mode.
            // To avoid this, we use a local variable to store the return value.
            int success = ::glfwInit();
            AW_CORE_ASSERT(success, "Could not initialize GLFW!");

            ::glfwSetErrorCallback(
                [](int error, const char* description)
                {
                    AW_CORE_ERROR("GLFW error ({0}): {1}", error, description);
                }
            );

            s_GLFWInitialized = true;
        }

        ::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // We use Vulkan, so disable OpenGL API
        ::glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);  // Resizing is not supported yet
        ::glfwMakeContextCurrent(m_Handle);

        m_Handle = ::glfwCreateWindow(m_Data.width, m_Data.height, m_Data.title.c_str(), nullptr, nullptr);
        AW_CORE_INFO("Created window {0} ({1}, {2})", m_Data.title, m_Data.width, m_Data.height);

        m_Context = GraphicsContext::create();
        m_Context->init();

        //::glfwMakeContextCurrent(m_Window);
        ::glfwSetWindowUserPointer(m_Handle, &m_Data);

        this->setVSync(m_Data.vsync);

        // Set GLFW callbacks >>>
        ::glfwSetWindowSizeCallback(m_Handle, [](GLFWwindow* window, int width, int height)
        {
            WindowData& data = *(WindowData*)::glfwGetWindowUserPointer(window);
            data.width = width;
            data.height = height;

            WindowResizeEvent evnt(width, height);
            data.eventCallback(evnt);
        });

        ::glfwSetWindowCloseCallback(m_Handle, [](GLFWwindow* window)
        {
            WindowData& data = *(WindowData*)::glfwGetWindowUserPointer(window);
            WindowCloseEvent evnt;
            data.eventCallback(evnt);
        });

        ::glfwSetKeyCallback(m_Handle, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            WindowData& data = *(WindowData*)::glfwGetWindowUserPointer(window);

            switch (action)
            {
                case GLFW_PRESS:
                {
                    KeyPressedEvent evnt((Key)key, 0);
                    data.eventCallback(evnt);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent evnt((Key)key);
                    data.eventCallback(evnt);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent evnt((Key)key, 1);
                    data.eventCallback(evnt);
                    break;
                }
            }
        });

        ::glfwSetMouseButtonCallback(m_Handle, [](GLFWwindow* window, int button, int action, int mods)
        {
            WindowData& data = *(WindowData*)::glfwGetWindowUserPointer(window);

            switch (action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent evnt((MouseButton)button);
                    data.eventCallback(evnt);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent evnt((MouseButton)button);
                    data.eventCallback(evnt);
                    break;
                }
            }
        });

        ::glfwSetScrollCallback(m_Handle, [](GLFWwindow* window, double xOffset, double yOffset)
        {
            WindowData& data = *(WindowData*)::glfwGetWindowUserPointer(window);

            MouseScrolledEvent evnt((float)xOffset, (float)yOffset);
            data.eventCallback(evnt);
        });

        ::glfwSetCursorPosCallback(m_Handle, [](GLFWwindow* window, double xPos, double yPos)
        {
            WindowData& data = *(WindowData*)::glfwGetWindowUserPointer(window);

            MouseMovedEvent evnt((float)xPos, (float)yPos);
            data.eventCallback(evnt);
        });
        // <<< Set GLFW callbacks
    }

    void WindowsWindow::destroy()
    {
        m_Context->destroy();
        m_Context = nullptr;

        ::glfwDestroyWindow(m_Handle);

        if (s_GLFWInitialized)
        {
            ::glfwTerminate();
            s_GLFWInitialized = false;
        }
    }

    void WindowsWindow::onUpdate()
    {
        ::glfwPollEvents();

        m_Context->swapBuffers();
    }

    void WindowsWindow::setVSync(bool enable)
    {
        ::glfwSwapInterval(enable ? 1 : 0);  // [NOTE] 0: disable, 1: enable (vsync)
        m_Data.vsync = enable;
    }

    MouseButtonState WindowsWindow::getMouseButtonState(MouseButton button)
    {
        int state = ::glfwGetMouseButton(m_Handle, (int)button);

        switch (state)
        {
            case GLFW_PRESS: { return MouseButtonState::Pressed; }
            case GLFW_RELEASE: { return MouseButtonState::Released; }
        }

        return MouseButtonState::None;
    }

    KeyState WindowsWindow::getKeyState(Key key)
    {
        int state = ::glfwGetKey(m_Handle, (int)key);

        switch (state)
        {
            case GLFW_PRESS: { return KeyState::Pressed; }
            case GLFW_RELEASE: { return KeyState::Released; }
            case GLFW_REPEAT: { return KeyState::Repeat; }
        }

        return KeyState::None;
    }

    glm::vec2 WindowsWindow::getCursorPosition()
    {
        double xPos, yPos;
        ::glfwGetCursorPos(m_Handle, &xPos, &yPos);

        return glm::vec2{ (float)xPos, (float)yPos };
    }

    void WindowsWindow::setCursorMode(CursorMode mode)
    {
        ::glfwSetInputMode(m_Handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
    }
}
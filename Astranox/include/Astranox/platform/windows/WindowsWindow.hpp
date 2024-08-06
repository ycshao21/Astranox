#pragma once

#include "Astranox/core/Window.hpp"

#include "Astranox/rendering/GraphicsContext.hpp"

struct GLFWwindow;

namespace Astranox
{
    class WindowsWindow final: public Window
    {
    public:
        WindowsWindow(const WindowSpecification& spec);
        virtual ~WindowsWindow() = default;

        void init() override;
        void destroy() override;

    public:
        void onResize(uint32_t width, uint32_t height) override;

        void beginFrame() override;

        void pollEvents() override;
        void swapBuffers() override;

        void setVSync(bool enable) override;
        bool isVSync() const override { return m_Data.vsync; }

        void setEventCallback(const EventCallbackFn& callback) override { m_Data.eventCallback = callback; }

    public: // Getters
        void* getHandle() override { return m_Handle; }

        const std::string& getTitle() const override { return m_Data.title; }
        uint32_t getWidth() const override { return m_Data.width; }
        uint32_t getHeight() const override { return m_Data.height; }
        std::pair<uint32_t, uint32_t> getSize() const override { return { (uint32_t)m_Data.width, (uint32_t)m_Data.height }; }

        Ref<GraphicsContext> getGraphicsContext() const { return m_Context; }

    private: // Input
        virtual MouseButtonState getMouseButtonState(MouseButton button) override;
        virtual KeyState getKeyState(Key key) override;
        virtual glm::vec2 getCursorPosition() override;

        virtual void setCursorMode(CursorMode mode) override;

    private:
        GLFWwindow* m_Handle = nullptr;

        // This structure can be passed directly to GLFW as a user pointer
        struct WindowData final
        {
            std::string title;
            uint32_t width;
            uint32_t height;

            bool vsync;
            EventCallbackFn eventCallback;
        };
        WindowData m_Data;

        Ref<GraphicsContext> m_Context = nullptr;
    };

}

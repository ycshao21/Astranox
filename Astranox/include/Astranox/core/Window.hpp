#pragma once
#include "pch.hpp"
#include "Input.hpp"

namespace Astranox
{
    struct WindowSpecification final
    {
        std::string title = "Astranox";
        uint32_t width = 1440;
        uint32_t height = 900;
        bool vsync = true;
    };

    /**
     * This is a platform-independent window interface.
     */
    class Window
    {
        friend class Input;

    public:
        using EventCallbackFn = std::function<void(Event&)>;

    public:
        static std::unique_ptr<Window> create(const WindowSpecification& spec);
        virtual ~Window() = default;

        virtual void init() = 0;
        virtual void destroy() = 0;

    public:
        virtual void onResize(uint32_t width, uint32_t height) = 0;

        virtual void pollEvents() = 0;

        virtual void beginFrame() = 0;
        virtual void swapBuffers() = 0;

    public:
        virtual void* getHandle() = 0;

        virtual const std::string& getTitle() const = 0;
        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;
        virtual std::pair<uint32_t, uint32_t> getSize() const = 0;

        virtual void setVSync(bool enable) = 0;
        virtual inline bool isVSync() const = 0;

        virtual void setEventCallback(const EventCallbackFn& callback) = 0;

    private:
        virtual MouseButtonState getMouseButtonState(MouseButton button) = 0;
        virtual KeyState getKeyState(Key key) = 0;
        virtual glm::vec2 getCursorPosition() = 0;

        virtual void setCursorMode(CursorMode mode) = 0;
    };
}

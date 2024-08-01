#include "pch.hpp"
#include "Astranox/core/Application.hpp"

namespace Astranox
{
    Application::Application(const ApplicationSpecification& spec)
    {
        s_Instance = this;

        WindowSpecification windowSpec;
        windowSpec.title = spec.name;
        windowSpec.width = spec.windowWidth;
        windowSpec.height = spec.windowHeight;
        windowSpec.vsync = spec.vsync;

        m_Window = Window::create(windowSpec);
        // [Vulkan] The creation of Vulkan surface requires the window to be initialized first.
        // So we call init() explicitly here.
        m_Window->init();
        m_Window->setEventCallback(
            [this](Event& e) {
                onEvent(e);
            }
        );
    }

    Application::~Application()
    {
        m_LayerStack.clear();

        m_Window->destroy();
        m_Window.reset();

        s_Instance = nullptr;
    }

    void Application::run()
    {
        while (m_Running)
        {
            // Update all layers
            for (Layer* layer : m_LayerStack)
            {
                layer->onUpdate();
            }

            // Update the window
            m_Window->onUpdate();
        }
    }

    void Application::onEvent(Event& evnt)
    {
        EventDispatcher dispatcher(evnt);
        dispatcher.dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return onWindowClose(e); });

        //AST_CORE_TRACE("{0}", evnt);

        // Propagate the event to all layers
        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            (*it)->onEvent(evnt);
            if (evnt.handled)
            {
                break;
            }
        }
    }

    void Application::pushLayer(Layer* layer)
    {
        m_LayerStack.pushLayer(layer);
    }

    void Application::pushOverlay(Layer* overlay)
    {
        m_LayerStack.pushOverlay(overlay);
    }

    bool Application::onWindowClose(WindowCloseEvent& e)
    {
        m_Running = false;
        return true;
    }
}

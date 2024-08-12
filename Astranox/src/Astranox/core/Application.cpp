#include "pch.hpp"
#include "Astranox/core/Application.hpp"
#include "Astranox/rendering/Renderer.hpp"

#include <GLFW/glfw3.h>

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

        Renderer::init();
    }

    Application::~Application()
    {
        m_LayerStack.clear();

        Renderer::shutdown();

        m_Window->destroy();
        m_Window.reset();

        s_Instance = nullptr;
    }

    void Application::run()
    {
        while (m_Running)
        {
            // [TODO] Platform::getTime()
            float time = static_cast<float>(glfwGetTime());
            Timestep timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;
            //AST_CORE_DEBUG("Frame time: {0}ms", timestep.getMilliseconds());

            m_Window->pollEvents();

            float startTime = std::chrono::duration<float>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            if (!m_Minimized)
            {
                m_Window->beginFrame();

                // Update all layers
                for (Layer* layer : m_LayerStack)
                {
                    layer->onUpdate(timestep);
                }

                // Update the window
                m_Window->swapBuffers();
            }
        }
    }

    void Application::onEvent(Event& evnt)
    {
        EventDispatcher dispatcher(evnt);
        dispatcher.dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return onWindowClose(e); });
        dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return onWindowResize(e); });

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

    bool Application::onWindowResize(WindowResizeEvent& e)
    {
        if (e.getWidth() == 0 || e.getHeight() == 0)
        {
            AST_CORE_WARN("Window minimized.");
            m_Minimized = true;
            return false;
        }

        m_Minimized = false;
        m_Window->onResize(e.getWidth(), e.getHeight());

        return false;
    }
}

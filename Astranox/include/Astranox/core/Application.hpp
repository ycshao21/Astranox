#pragma once
#include "Window.hpp"
#include "LayerStack.hpp"
#include "events/ApplicationEvent.hpp"

#include "Astranox/platform/vulkan/VulkanRenderer.hpp"

namespace Astranox
{

    struct ApplicationSpecification final
    {
        std::string name = "Astranox";
        uint32_t windowWidth = 1440;
        uint32_t windowHeight = 900;
        bool vsync = true;
    };

    /**
     * A base class for all applications.
     */
    class Application
    {
    public:
        Application(const ApplicationSpecification& spec = {});
        virtual ~Application();

        /**
         * Run the application.
         * This function is called by the engine.
         */
        virtual void run() final;

        virtual void onEvent(Event& evnt) final;

    public: // Getters
        inline static Application& get() { return *s_Instance; }
        inline virtual Window& getWindow() const final { return *m_Window; }

    public: // Layer management
        virtual void pushLayer(Layer* layer) final;
        virtual void pushOverlay(Layer* overlay) final;

    private: // Event handling
        virtual bool onWindowClose(WindowCloseEvent& e) final;

    private:
        inline static Application* s_Instance = nullptr;
        bool m_Running = true;
        std::unique_ptr<Window> m_Window = nullptr;
        LayerStack m_LayerStack;

        // TEMP
        Ref<VulkanRenderer> m_Renderer = nullptr;
    };

    /**
     * To develop your own application, you need to:
     *  (1) Inherit from the `Application` class.
     *  (2) Implement the `createApplication` function in your own project (simply return a pointer to your application instance).
     */
    Application* createApplication();

}

#pragma once
#include <Astranox.hpp>

class Sandbox2DLayer: public Astranox::Layer
{
public:
    Sandbox2DLayer() : Layer("Sandbox2DLayer")
    {
        m_Camera = Astranox::Ref<Astranox::PerspectiveCamera>::create(75.0f, 0.1f, 100.0f);

        std::filesystem::path texturePath = "assets/textures/statue.jpg";
        m_Texture = Astranox::Texture2D::create(texturePath);
    }

    virtual void onAttach() override
    {
        m_Renderer2D = Astranox::Ref<Astranox::Renderer2D>::create();
        m_Renderer2D->init();
    }

    virtual void onDetach() override
    {
        m_Texture = nullptr;

        m_Renderer2D->shutdown();
    }

    virtual void onUpdate(Astranox::Timestep ts)
    {
        m_Camera->onUpdate(ts);
        auto& window = Astranox::Application::get().getWindow();
        m_Camera->onResize(window.getWidth(), window.getHeight());

        m_Renderer2D->resetStats();

        m_Renderer2D->beginScene(*m_Camera);
        {
            for (float y = -10.0f; y < 10.0f; y += 0.5f)
            {
                for (float x = -10.0f; x < 10.0f; x += 0.5f)
                {
                    glm::vec4 color = { (x + 10.0f) / 20.0f, 0.4f, (y + 10.0f) / 20.0f, 1.0f };
                    m_Renderer2D->drawQuad({ x, y, -8.0f }, { 0.45f, 0.45f }, color);
                }
            }

            m_Renderer2D->drawQuad({ -1.8f, 0.4f, -3.0f }, { 2.2f, 2.5f }, { 0.2f, 0.3f, 0.8f, 1.0f });
            m_Renderer2D->drawRotatedQuad({ 1.9f, -0.7f, 0.0f }, { 1.4f, 1.8f }, -30.0f, { 0.3f, 0.8f, 0.2f, 1.0f });
            m_Renderer2D->drawRotatedQuad({ 0.0f, 0.0f, -1.0f }, { 3.0f, 3.0f }, degrees, { 0.8f, 0.2f, 0.3f, 0.7f });
            //m_Renderer2D->drawQuad({ 0.4f, 0.4f, -1.5f }, { 1.8f, 1.8f }, m_Texture);
        }
        m_Renderer2D->endScene();

        degrees += delta * ts;
    }

    virtual void onEvent(Astranox::Event& event) override
    {
    }

private:
    Astranox::Ref<Astranox::Renderer2D> m_Renderer2D;

    Astranox::ShaderLibrary m_ShaderLibrary;
    Astranox::Ref<Astranox::PerspectiveCamera> m_Camera = nullptr;

    Astranox::Ref<Astranox::Texture2D> m_Texture;

    float degrees = 0.0f;
    float delta = 100.0f;
};



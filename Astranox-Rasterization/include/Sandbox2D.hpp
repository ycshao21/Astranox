#pragma once
#include <Astranox.hpp>

class Sandbox2DLayer: public Astranox::Layer
{
public:
    Sandbox2DLayer() : Layer("Sandbox2DLayer")
    {
        m_Camera = Astranox::Ref<Astranox::PerspectiveCamera>::create(45.0f, 0.1f, 100.0f);

        std::filesystem::path texturePath = "../Astranox-Rasterization/assets/textures/statue.jpg";
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

        m_Renderer2D->beginScene(*m_Camera);

        m_Renderer2D->drawQuad({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.8f, 0.2f, 0.3f, 0.5f });
        m_Renderer2D->drawQuad({ 0.7f, -0.7f, -1.0f }, { 1.5f, 2.5f }, { 0.2f, 0.3f, 0.8f, 0.5f });
        m_Renderer2D->drawQuad({ -0.7f, 0.7f, -0.5f }, { 1.4f, 0.7f }, { 0.3f, 0.8f, 0.2f, 0.5f });
        //m_Renderer2D->drawQuad({ 0.4f, 0.4f, -1.5f }, { 1.8f, 1.8f }, m_Texture);

        m_Renderer2D->endScene();
    }

    virtual void onEvent(Astranox::Event& event) override
    {
    }

private:
    Astranox::Ref<Astranox::Renderer2D> m_Renderer2D;

    Astranox::ShaderLibrary m_ShaderLibrary;
    Astranox::Ref<Astranox::PerspectiveCamera> m_Camera = nullptr;

    Astranox::Ref<Astranox::Texture2D> m_Texture;
};



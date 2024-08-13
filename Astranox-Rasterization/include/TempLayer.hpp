#pragma once
#include <Astranox.hpp>

struct CameraData
{
    alignas(16) glm::mat4 viewProjection;
};

class TempLayer : public Astranox::Layer
{
public:
    TempLayer() : Layer("TempLayer")
    {
        m_Camera = Astranox::Ref<Astranox::PerspectiveCamera>::create(45.0f, 0.1f, 100.0f);
    }

    virtual void onAttach() override
    {
        std::filesystem::path vertexShaderPath = "../Astranox-Rasterization/assets/shaders/Texture-Vert.spv";
        std::filesystem::path fragmentShaderPath = "../Astranox-Rasterization/assets/shaders/Texture-Frag.spv";
        auto shader = m_ShaderLibrary.load("Texture", vertexShaderPath, fragmentShaderPath);
        shader.as<Astranox::VulkanShader>()->createDescriptorSetLayout();

        Astranox::VertexBufferLayout vertexBufferLayout{
            {Astranox::ShaderDataType::Vec3, "a_Position"},
            {Astranox::ShaderDataType::Vec4, "a_Color"},
            {Astranox::ShaderDataType::Vec2, "a_TexCoord"}
        };

        m_Pipeline = Astranox::Ref<Astranox::VulkanPipeline>::create(shader, vertexBufferLayout);
        m_Pipeline->createPipeline();

        std::filesystem::path texturePath = "../Astranox-Rasterization/assets/textures/viking_room.png";
        m_Texture = Astranox::Texture::create(texturePath, true);

        std::filesystem::path meshPath = "../Astranox-Rasterization/assets/models/viking_room.obj";
        m_RoomMesh = Astranox::readMesh(meshPath);

        m_CameraUBA = Astranox::UniformBufferArray::create(sizeof(CameraData));

        m_DescriptorManager = Astranox::Ref<Astranox::VulkanDescriptorManager>::create();
        m_DescriptorManager->init(
            shader.as<Astranox::VulkanShader>()->getDescriptorSetLayouts()[0],
            m_Texture.as<Astranox::VulkanTexture>()->getSampler(),
            m_Texture.as<Astranox::VulkanTexture>()->getImageView(),
            m_CameraUBA
        );
    }

    virtual void onDetach() override
    {
        auto device = Astranox::VulkanContext::get()->getDevice();
        device->waitIdle();

        m_CameraUBA = nullptr;
        m_Texture = nullptr;
        m_Pipeline = nullptr;

        m_DescriptorManager = nullptr;
    }

    virtual void onUpdate(Astranox::Timestep ts)
    {
        // [TODO] Move this to a separate function
        m_Camera->onUpdate(ts);
        auto& window = Astranox::Application::get().getWindow();
        m_Camera->onResize(window.getWidth(), window.getHeight());


        auto swapchain = Astranox::VulkanContext::get()->getSwapchain();

        Astranox::Renderer::beginFrame();
        Astranox::Renderer::beginRenderPass(
            swapchain->getCurrentCommandBuffer(),
            swapchain->getRenderPass(),
            m_Pipeline,
            m_DescriptorManager->getDescriptorSets()
        );

        CameraData cameraData{
            .viewProjection = m_Camera->getProjectionMatrix() * m_Camera->getViewMatrix()
        };
        m_CameraUBA->getCurrentBuffer()->setData(&cameraData, sizeof(CameraData), 0);

        Astranox::Renderer::renderMesh(swapchain->getCurrentCommandBuffer(), m_Pipeline, m_RoomMesh, 1);

        // End render pass
        Astranox::Renderer::endRenderPass(swapchain->getCurrentCommandBuffer());
        Astranox::Renderer::endFrame();
    }

    virtual void onEvent(Astranox::Event& event) override
    {
    }

private:
    Astranox::ShaderLibrary m_ShaderLibrary;
    Astranox::Ref<Astranox::VulkanPipeline> m_Pipeline = nullptr;

    Astranox::Ref<Astranox::PerspectiveCamera> m_Camera = nullptr;

    Astranox::Ref<Astranox::Texture> m_Texture = nullptr;

    Astranox::Mesh m_RoomMesh;

    Astranox::Ref<Astranox::UniformBufferArray> m_CameraUBA = nullptr;

    Astranox::Ref<Astranox::VulkanDescriptorManager> m_DescriptorManager = nullptr;
};


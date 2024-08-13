#pragma once
#include <Astranox.hpp>

class SandboxLayer: public Astranox::Layer
{
public:
    SandboxLayer() : Layer("TempLayer")
    {
        m_Camera = Astranox::Ref<Astranox::PerspectiveCamera>::create(45.0f, 0.1f, 100.0f);
    }

    virtual void onAttach() override
    {
        std::filesystem::path vertexShaderPath = "../Astranox-Rasterization/assets/shaders/PureColor-Vert.spv";
        std::filesystem::path fragmentShaderPath = "../Astranox-Rasterization/assets/shaders/PureColor-Frag.spv";
        auto shader = m_ShaderLibrary.load("PureColor", vertexShaderPath, fragmentShaderPath);
        shader.as<Astranox::VulkanShader>()->createDescriptorSetLayout();

        Astranox::VertexBufferLayout vertexBufferLayout{
            {Astranox::ShaderDataType::Vec3, "a_Position"},
            {Astranox::ShaderDataType::Vec4, "a_Color"},
        };

        m_Pipeline = Astranox::Ref<Astranox::VulkanPipeline>::create(shader, vertexBufferLayout);
        m_Pipeline->createPipeline();

        // [NOTE] Don't remove this! It will cause a crash.
        std::filesystem::path texturePath = "../Astranox-Rasterization/assets/textures/viking_room.png";
        m_Texture = Astranox::Texture::create(texturePath, true);


        // Vertex data for a cube
        std::vector<Astranox::Vertex> cubeVertices = {
            // Front face
            {{-0.5f, -0.5f,  0.5f}, {0.9f, 0.4f, 0.4f, 1.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.9f, 0.4f, 0.4f, 1.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.9f, 0.4f, 0.4f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, {0.9f, 0.4f, 0.4f, 1.0f}},
            
            // Back face
            {{ 0.5f, -0.5f, -0.5f}, {0.4f, 0.8f, 0.4f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.4f, 0.8f, 0.4f, 1.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.4f, 0.8f, 0.4f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.4f, 0.8f, 0.4f, 1.0f}},

            // Left face
            {{-0.5f, -0.5f, -0.5f}, {0.4f, 0.4f, 0.8f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f}, {0.4f, 0.4f, 0.8f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, {0.4f, 0.4f, 0.8f, 1.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.4f, 0.4f, 0.8f, 1.0f}},

            // Right face
            {{ 0.5f, -0.5f,  0.5f}, {0.9f, 0.9f, 0.6f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {0.9f, 0.9f, 0.6f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.9f, 0.9f, 0.6f, 1.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.9f, 0.9f, 0.6f, 1.0f}},

            // Top face
            {{-0.5f,  0.5f,  0.5f}, {0.6f, 0.9f, 0.9f, 1.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.6f, 0.9f, 0.9f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.6f, 0.9f, 0.9f, 1.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.6f, 0.9f, 0.9f, 1.0f}},

            // Bottom face
            {{-0.5f, -0.5f, -0.5f}, {0.9f, 0.6f, 0.9f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {0.9f, 0.6f, 0.9f, 1.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.9f, 0.6f, 0.9f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f}, {0.9f, 0.6f, 0.9f, 1.0f}}
        };

        std::vector<uint32_t> cubeIndices = {
            // Front face
            0, 1, 2, 2, 3, 0,
            // Back face
            4, 5, 6, 6, 7, 4,
            // Left face
            8, 9, 10, 10, 11, 8,
            // Right face
            12, 13, 14, 14, 15, 12,
            // Top face
            16, 17, 18, 18, 19, 16,
            // Bottom face
            20, 21, 22, 22, 23, 20
        };
        m_CubeMesh = Astranox::Mesh(cubeVertices, cubeIndices);

        std::vector<Astranox::Vertex> tetrahedronVertices = {
            {{2.0f, sqrt(2.0f / 3.0f), 0.0f}, {0.95f, 0.77f, 0.52f, 1.0f}},
            {{1.5f, 0.0f, -sqrt(3.0f) * 0.5f}, {0.67f, 0.85f, 0.91f, 1.0f}},
            {{2.5f, 0.0f, -sqrt(3.0f) * 0.5f}, {0.82f, 0.56f, 0.74f, 1.0f}},
            {{2.0f, 0.0f, sqrt(3.0f) * 0.5f}, {0.59f, 0.44f, 0.84f, 1.0f}}
        };

        std::vector<uint32_t> tetrahedronIndices = {
            0, 1, 2,
            0, 2, 3,
            0, 3, 1,
            1, 3, 2
        };

        m_TetrahedronMesh = Astranox::Mesh(tetrahedronVertices, tetrahedronIndices);


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

        CameraData cameraData;
        cameraData.viewProjection = m_Camera->getProjectionMatrix() * m_Camera->getViewMatrix();
        m_CameraUBA->getCurrentBuffer()->setData(&cameraData, sizeof(CameraData), 0);

        Astranox::Renderer::renderMesh(swapchain->getCurrentCommandBuffer(), m_Pipeline, m_CubeMesh, 1);
        Astranox::Renderer::renderMesh(swapchain->getCurrentCommandBuffer(), m_Pipeline, m_TetrahedronMesh, 1);

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

    Astranox::Mesh m_CubeMesh;
    Astranox::Mesh m_TetrahedronMesh;

    Astranox::Ref<Astranox::UniformBufferArray> m_CameraUBA = nullptr;

    Astranox::Ref<Astranox::VulkanDescriptorManager> m_DescriptorManager = nullptr;
};



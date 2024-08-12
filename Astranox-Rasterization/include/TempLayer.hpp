#pragma once
#include <Astranox.hpp>

struct SceneData
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
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
        std::filesystem::path vertexShaderPath = "../Astranox-Rasterization/assets/shaders/square-vert.spv";
        std::filesystem::path fragmentShaderPath = "../Astranox-Rasterization/assets/shaders/square-frag.spv";
        m_Shader = Astranox::VulkanShader::create(vertexShaderPath, fragmentShaderPath);
        m_Shader->createDescriptorSetLayout();

        m_Pipeline = Astranox::Ref<Astranox::VulkanPipeline>::create(m_Shader);
        m_Pipeline->createPipeline();

        std::filesystem::path texturePath = "../Astranox-Rasterization/assets/textures/viking_room.png";
        m_Texture = Astranox::Texture::create(texturePath, true);

        std::filesystem::path meshPath = "../Astranox-Rasterization/assets/models/viking_room.obj";
        m_RoomMesh = Astranox::readMesh(meshPath);


        std::vector<Astranox::Vertex> cubeVertices{
            // ����
            { {-1.0f, -1.0f, -1.0f}, {0.2f, 0.3f, 0.7f, 1.0f}, {0.0f, 0.0f} }, // ���º�
            { { 1.0f, -1.0f, -1.0f}, {0.2f, 0.3f, 0.7f, 1.0f}, {1.0f, 0.0f} }, // ���º�
            { { 1.0f,  1.0f, -1.0f}, {0.2f, 0.3f, 0.7f, 1.0f}, {1.0f, 1.0f} }, // ���Ϻ�
            { { 1.0f,  1.0f, -1.0f}, {0.2f, 0.3f, 0.7f, 1.0f}, {1.0f, 1.0f} }, // ���Ϻ�
            { {-1.0f,  1.0f, -1.0f}, {0.2f, 0.3f, 0.7f, 1.0f}, {0.0f, 1.0f} }, // ���Ϻ�
            { {-1.0f, -1.0f, -1.0f}, {0.2f, 0.3f, 0.7f, 1.0f}, {0.0f, 0.0f} }, // ���º�

            // ǰ��
            { {-1.0f, -1.0f,  1.0f}, {0.7f, 0.2f, 0.3f, 1.0f}, {0.0f, 0.0f} }, // ����ǰ
            { { 1.0f, -1.0f,  1.0f}, {0.7f, 0.2f, 0.3f, 1.0f}, {1.0f, 0.0f} }, // ����ǰ
            { { 1.0f,  1.0f,  1.0f}, {0.7f, 0.2f, 0.3f, 1.0f}, {1.0f, 1.0f} }, // ����ǰ
            { { 1.0f,  1.0f,  1.0f}, {0.7f, 0.2f, 0.3f, 1.0f}, {1.0f, 1.0f} }, // ����ǰ
            { {-1.0f,  1.0f,  1.0f}, {0.7f, 0.2f, 0.3f, 1.0f}, {0.0f, 1.0f} }, // ����ǰ
            { {-1.0f, -1.0f,  1.0f}, {0.7f, 0.2f, 0.3f, 1.0f}, {0.0f, 0.0f} }, // ����ǰ

            // ����
            { {-1.0f,  1.0f,  1.0f}, {0.2f, 0.7f, 0.3f, 1.0f}, {1.0f, 0.0f} }, // ����ǰ
            { {-1.0f,  1.0f, -1.0f}, {0.2f, 0.7f, 0.3f, 1.0f}, {1.0f, 1.0f} }, // ���Ϻ�
            { {-1.0f, -1.0f, -1.0f}, {0.2f, 0.7f, 0.3f, 1.0f}, {0.0f, 1.0f} }, // ���º�
            { {-1.0f, -1.0f, -1.0f}, {0.2f, 0.7f, 0.3f, 1.0f}, {0.0f, 1.0f} }, // ���º�
            { {-1.0f, -1.0f,  1.0f}, {0.2f, 0.7f, 0.3f, 1.0f}, {0.0f, 0.0f} }, // ����ǰ
            { {-1.0f,  1.0f,  1.0f}, {0.2f, 0.7f, 0.3f, 1.0f}, {1.0f, 0.0f} }, // ����ǰ

            // ����
            { { 1.0f,  1.0f,  1.0f}, {0.7f, 0.7f, 0.2f, 1.0f}, {1.0f, 0.0f} }, // ����ǰ
            { { 1.0f,  1.0f, -1.0f}, {0.7f, 0.7f, 0.2f, 1.0f}, {1.0f, 1.0f} }, // ���Ϻ�
            { { 1.0f, -1.0f, -1.0f}, {0.7f, 0.7f, 0.2f, 1.0f}, {0.0f, 1.0f} }, // ���º�
            { { 1.0f, -1.0f, -1.0f}, {0.7f, 0.7f, 0.2f, 1.0f}, {0.0f, 1.0f} }, // ���º�
            { { 1.0f, -1.0f,  1.0f}, {0.7f, 0.7f, 0.2f, 1.0f}, {0.0f, 0.0f} }, // ����ǰ
            { { 1.0f,  1.0f,  1.0f}, {0.7f, 0.7f, 0.2f, 1.0f}, {1.0f, 0.0f} }, // ����ǰ

            // ����
            { {-1.0f, -1.0f, -1.0f}, {0.3f, 0.2f, 0.7f, 1.0f}, {0.0f, 1.0f} }, // ���º�
            { { 1.0f, -1.0f, -1.0f}, {0.3f, 0.2f, 0.7f, 1.0f}, {1.0f, 1.0f} }, // ���º�
            { { 1.0f, -1.0f,  1.0f}, {0.3f, 0.2f, 0.7f, 1.0f}, {1.0f, 0.0f} }, // ����ǰ
            { { 1.0f, -1.0f,  1.0f}, {0.3f, 0.2f, 0.7f, 1.0f}, {1.0f, 0.0f} }, // ����ǰ
            { {-1.0f, -1.0f,  1.0f}, {0.3f, 0.2f, 0.7f, 1.0f}, {0.0f, 0.0f} }, // ����ǰ
            { {-1.0f, -1.0f, -1.0f}, {0.3f, 0.2f, 0.7f, 1.0f}, {0.0f, 1.0f} }, // ���º�

            // ����
            { {-1.0f,  1.0f, -1.0f}, {0.7f, 0.3f, 0.2f, 1.0f}, {0.0f, 1.0f} }, // ���Ϻ�
            { { 1.0f,  1.0f, -1.0f}, {0.7f, 0.3f, 0.2f, 1.0f}, {1.0f, 1.0f} }, // ���Ϻ�
            { { 1.0f,  1.0f,  1.0f}, {0.7f, 0.3f, 0.2f, 1.0f}, {1.0f, 0.0f} }, // ����ǰ
            { { 1.0f,  1.0f,  1.0f}, {0.7f, 0.3f, 0.2f, 1.0f}, {1.0f, 0.0f} }, // ����ǰ
            { {-1.0f,  1.0f,  1.0f}, {0.7f, 0.3f, 0.2f, 1.0f}, {0.0f, 0.0f} }, // ����ǰ
            { {-1.0f,  1.0f, -1.0f}, {0.7f, 0.3f, 0.2f, 1.0f}, {0.0f, 1.0f} }  // ���Ϻ�
        };

        std::vector<Astranox::Index> cubeIndices{
            // ����
            0, 1, 2,    // ��һ��������
            2, 3, 0,    // �ڶ���������

            // ǰ��
            4, 5, 6,    // ��һ��������
            6, 7, 4,    // �ڶ���������

            // ����
            8, 9, 10,   // ��һ��������
            10, 11, 8,  // �ڶ���������

            // ����
            12, 13, 14, // ��һ��������
            14, 15, 12, // �ڶ���������

            // ����
            16, 17, 18, // ��һ��������
            18, 19, 16, // �ڶ���������

            // ����
            20, 21, 22, // ��һ��������
            22, 23, 20  // �ڶ���������
        };
        m_CubeMesh = Astranox::Mesh(cubeVertices, cubeIndices);


        m_SceneUBA = Astranox::UniformBufferArray::create(sizeof(SceneData));

        m_DescriptorManager = Astranox::Ref<Astranox::VulkanDescriptorManager>::create();
        m_DescriptorManager->init(
            m_Shader->getDescriptorSetLayouts()[0],
            m_Texture.as<Astranox::VulkanTexture>()->getSampler(),
            m_Texture.as<Astranox::VulkanTexture>()->getImageView(),
            m_SceneUBA
        );
    }

    virtual void onDetach() override
    {
        auto device = Astranox::VulkanContext::get()->getDevice();
        device->waitIdle();

        m_SceneUBA = nullptr;
        m_Texture = nullptr;
        m_Pipeline = nullptr;

        m_DescriptorManager = nullptr;
        m_Shader = nullptr;
    }

    virtual void onUpdate(Astranox::Timestep ts)
    {
        // [TODO] Move this to a separate function
        m_Camera->onUpdate(ts);
        auto& window = Astranox::Application::get().getWindow();
        m_Camera->onResize(window.getWidth(), window.getHeight());

        // [TODO] Move this to Renderer->BeginScene()
        // Update uniform buffer
        SceneData sceneData;
        sceneData.model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        sceneData.projection = m_Camera->getProjectionMatrix();
        sceneData.projection[1][1] *= -1.0f; // Flip the Y axis for Vulkan [0, height] to [height, 0]
        sceneData.view = m_Camera->getViewMatrix();

        m_SceneUBA->getCurrentBuffer()->setData(&sceneData);

        render();
    }

    void render()
    {
        auto swapchain = Astranox::VulkanContext::get()->getSwapchain();

        m_Renderer->beginFrame();
        m_Renderer->beginRenderPass(
            swapchain->getCurrentCommandBuffer(),
            swapchain->getRenderPass(),
            m_Pipeline,
            m_DescriptorManager->getDescriptorSets()
        );

        m_Renderer->render(swapchain->getCurrentCommandBuffer(), m_Pipeline, m_RoomMesh.getVertexBuffer(), m_RoomMesh.getIndexBuffer(), 1);
        //m_Renderer->render(swapchain->getCurrentCommandBuffer(), m_Pipeline, m_CubeMesh.getVertexBuffer(), m_CubeMesh.getIndexBuffer(), 1);

        // End render pass
        m_Renderer->endRenderPass(swapchain->getCurrentCommandBuffer());
        m_Renderer->endFrame();
    }

    virtual void onEvent(Astranox::Event& event) override
    {
    }

private:
    Astranox::Ref<Astranox::VulkanRenderer> m_Renderer = nullptr;

    Astranox::Ref<Astranox::VulkanShader> m_Shader = nullptr;
    Astranox::Ref<Astranox::VulkanPipeline> m_Pipeline = nullptr;

    Astranox::Ref<Astranox::PerspectiveCamera> m_Camera = nullptr;

    Astranox::Ref<Astranox::Texture> m_Texture = nullptr;

    Astranox::Mesh m_RoomMesh;
    Astranox::Mesh m_CubeMesh;

    Astranox::Ref<Astranox::UniformBufferArray> m_SceneUBA = nullptr;

    Astranox::Ref<Astranox::VulkanDescriptorManager> m_DescriptorManager = nullptr;
};


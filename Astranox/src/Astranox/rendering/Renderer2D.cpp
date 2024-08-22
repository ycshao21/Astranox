#include "pch.hpp"
#include "Astranox/rendering/Renderer2D.hpp"

#include "Astranox/rendering/Shader.hpp"
#include "Astranox/rendering/Renderer.hpp"

#include "Astranox/rendering/VertexBufferLayout.hpp"
#include "Astranox/rendering/Texture.hpp"
#include "Astranox/rendering/UniformBufferArray.hpp"

#include "Astranox/platform/vulkan/VulkanShader.hpp"
#include "Astranox/platform/vulkan/VulkanPipeline.hpp"
#include "Astranox/platform/vulkan/VulkanDescriptorManager.hpp"
#include "Astranox/platform/vulkan/VulkanTexture.hpp"

namespace Astranox
{
    constexpr glm::vec4 s_QuadVertexPositions[] = {
        { -0.5f, -0.5f, 0.0f, 1.0f }, 
        {  0.5f, -0.5f, 0.0f, 1.0f },
        {  0.5f,  0.5f, 0.0f, 1.0f },
        { -0.5f,  0.5f, 0.0f, 1.0f }
    };

    struct QuadVertex
    {
        glm::vec3 position;
        glm::vec4 color;
    };

    struct Renderer2DData
    {
        const uint32_t maxQuads = 10000;
        const uint32_t maxVertices = maxQuads * 4;
        const uint32_t maxIndices = maxQuads * 6;

        Ref<VulkanPipeline> pipeline;
        Ref<Shader> shader;
        Ref<VulkanDescriptorManager> descriptorManager;
        Ref<UniformBufferArray> cameraUBA;
        Ref<VertexBuffer> quadVB;
        Ref<IndexBuffer> quadIB;

        uint32_t quadIndexCount = 0;
        QuadVertex* quadVertexBufferBase = nullptr;
        QuadVertex* quadVertexBufferPtr = nullptr;
    };

    static Renderer2DData* s_Data = nullptr;

    void Renderer2D::init()
    {
        s_Data = new Renderer2DData;

        std::filesystem::path vertexShaderPath = "../Astranox-Rasterization/assets/shaders/PureColor-Vert.spv";
        std::filesystem::path fragmentShaderPath = "../Astranox-Rasterization/assets/shaders/PureColor-Frag.spv";

        std::map<uint32_t, UniformBufferInfo> uniformBufferInfos{
            { 0, { VK_SHADER_STAGE_VERTEX_BIT, "Camera" } },
        };
        s_Data->shader = Shader::create("PureColor", vertexShaderPath, fragmentShaderPath);
        s_Data->shader.as<VulkanShader>()->setUniformBufferInfos(uniformBufferInfos);
        s_Data->shader.as<VulkanShader>()->createDescriptorSetLayouts();


        VertexBufferLayout vertexBufferLayout{
            {ShaderDataType::Vec3, "a_Position"},
            {ShaderDataType::Vec4, "a_Color"}
        };
        s_Data->quadVB = VertexBuffer::create(s_Data->maxVertices * sizeof(QuadVertex));

        s_Data->quadVertexBufferBase = new QuadVertex[s_Data->maxVertices];

        Index* quadIndices = new Index[s_Data->maxIndices];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_Data->maxIndices; i += 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }
        s_Data->quadIB = IndexBuffer::create(quadIndices, s_Data->maxIndices * sizeof(Index));
        delete[] quadIndices;

        PipelineSpecification pipelineSpec{
            .shader = s_Data->shader,
            .vertexBufferLayout = vertexBufferLayout,
            .depthTestEnable = true,
            .depthWriteEnable = false
        };
        s_Data->pipeline = Ref<VulkanPipeline>::create(pipelineSpec);

        s_Data->cameraUBA = UniformBufferArray::create(sizeof(CameraData));

        s_Data->descriptorManager = Ref<VulkanDescriptorManager>::create();
        s_Data->descriptorManager->createDescriptorSets(s_Data->shader, s_Data->cameraUBA);
    }

    void Renderer2D::shutdown()
    {
        auto device = VulkanContext::get()->getDevice();
        device->waitIdle();

        delete s_Data;
    }

    void Renderer2D::beginScene(const PerspectiveCamera& camera)
    {
        // Upload view projection
        CameraData cameraData;
        cameraData.viewProjection = camera.getProjectionMatrix() * camera.getViewMatrix();
        s_Data->cameraUBA->getCurrentBuffer()->setData(&cameraData, sizeof(CameraData), 0);

        // Reset quad info
        s_Data->quadIndexCount = 0;
        s_Data->quadVertexBufferPtr = s_Data->quadVertexBufferBase;
    }

    void Renderer2D::endScene()
    {
        auto swapchain = VulkanContext::get()->getSwapchain();

        Renderer::beginFrame();

        // Quad rendering
        uint32_t quadDataSize = (uint32_t)((uint8_t*)s_Data->quadVertexBufferPtr - (uint8_t*)s_Data->quadVertexBufferBase);
        if (quadDataSize > 0)
        {
            s_Data->quadVB->setData(s_Data->quadVertexBufferBase, quadDataSize);

            Renderer::beginRenderPass(
                swapchain->getCurrentCommandBuffer(),
                swapchain->getRenderPass(),
                s_Data->pipeline,
                s_Data->descriptorManager->getDescriptorSets(Renderer::getCurrentFrameIndex())
            );

            Renderer::renderGeometry(
                swapchain->getCurrentCommandBuffer(),
                s_Data->pipeline,
                s_Data->quadVB,
                s_Data->quadIB,
                s_Data->quadIndexCount
            );

            Renderer::endRenderPass(swapchain->getCurrentCommandBuffer());
        }

        Renderer::endFrame();
    }

    void Renderer2D::drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        drawQuad({ position.x, position.y, 0.0f }, size, color);
    }

    void Renderer2D::drawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        constexpr size_t quadVertexCount = 4;
        for (size_t i = 0; i < quadVertexCount; ++i)
        {
            s_Data->quadVertexBufferPtr->position = transform * s_QuadVertexPositions[i];
            s_Data->quadVertexBufferPtr->color = color;
            s_Data->quadVertexBufferPtr++;
        }

        s_Data->quadIndexCount += 6;
    }
}

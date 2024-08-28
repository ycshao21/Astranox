#include "pch.hpp"
#include "Astranox/rendering/Renderer2D.hpp"

#include "Astranox/rendering/Shader.hpp"
#include "Astranox/rendering/Renderer.hpp"

#include "Astranox/rendering/VertexBufferLayout.hpp"
#include "Astranox/rendering/Texture2D.hpp"
#include "Astranox/rendering/UniformBufferArray.hpp"

#include "Astranox/platform/vulkan/VulkanShader.hpp"
#include "Astranox/platform/vulkan/VulkanPipeline.hpp"
#include "Astranox/platform/vulkan/VulkanDescriptorManager.hpp"
#include "Astranox/platform/vulkan/VulkanTexture2D.hpp"

namespace Astranox
{
    struct QuadVertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texCoord;
        float texIndex;
        float tilingFactor;
    };

    struct Renderer2DData
    {
        static const uint32_t maxQuads = 10000;
        static const uint32_t maxVertices = maxQuads * 4;
        static const uint32_t maxIndices = maxQuads * 6;
        static const uint32_t maxTextureSlots = 32;

        Ref<VulkanPipeline> pipeline;
        Ref<Shader> shader;
        Ref<VulkanDescriptorManager> descriptorManager;
        Ref<UniformBufferArray> cameraUBA;
        Ref<VertexBuffer> quadVB;
        Ref<IndexBuffer> quadIB;
        Ref<Texture2D> whiteTexture;

        uint32_t quadIndexCount = 0;
        QuadVertex* quadVertexBufferBase = nullptr;
        QuadVertex* quadVertexBufferPtr = nullptr;

        std::array<Ref<Texture2D>, maxTextureSlots> textureSlots;
        uint32_t textureSlotIndex = 1;  // 0: white texture

        glm::vec4 quadVertexPositions[4];

        Renderer2D::Statistics stats;
    };

    static Renderer2DData* s_Data = nullptr;

    void Renderer2D::init()
    {
        s_Data = new Renderer2DData;

        std::filesystem::path vertexShaderPath = "../Astranox-Rasterization/assets/shaders/Texture-Vert.spv";
        std::filesystem::path fragmentShaderPath = "../Astranox-Rasterization/assets/shaders/Texture-Frag.spv";

        ShaderDescriptorSetInfo sdsi;
        sdsi.uniformBufferInfos = {
            { 0, { 1, VK_SHADER_STAGE_VERTEX_BIT, "u_Camera" } },
        };
        sdsi.imageSamplerInfos = {
            { 1, { Renderer2DData::maxTextureSlots, VK_SHADER_STAGE_FRAGMENT_BIT, "u_Textures" }}
        };
        s_Data->shader = Shader::create("Renderer2D", vertexShaderPath, fragmentShaderPath);
        s_Data->shader.as<VulkanShader>()->setDescriptorSetInfo(sdsi);
        s_Data->shader.as<VulkanShader>()->createDescriptorSetLayouts();


        // Vertex buffer >>>
        VertexBufferLayout vertexBufferLayout{
            {ShaderDataType::Vec3, "a_Position"},
            {ShaderDataType::Vec4, "a_Color"},
            {ShaderDataType::Vec2, "a_TexCoord"},
            {ShaderDataType::Float, "a_TexIndex"},
            {ShaderDataType::Float, "a_TilingFactor"}
        };
        s_Data->quadVB = VertexBuffer::create(Renderer2DData::maxVertices * sizeof(QuadVertex));

        s_Data->quadVertexBufferBase = new QuadVertex[Renderer2DData::maxVertices];
        // <<< Vertex buffer

        // Index buffer >>>
        Index* quadIndices = new Index[Renderer2DData::maxIndices];

        uint32_t offset = 0;
        for (uint32_t i = 0; i < Renderer2DData::maxIndices; i += 6)
        {
            /*
             * [NOTE] Each quad is made up of two triangles, hence it has 4 vertices and 6 indices.
             *  3---2
             *  |  /|
             *  | / |
             *  |/  |
             *  0---1
             * Indices: 0, 1, 2, 2, 3, 0
            */
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }
        s_Data->quadIB = IndexBuffer::create(quadIndices, Renderer2DData::maxIndices * sizeof(Index));
        delete[] quadIndices;
        // <<< Index buffer

        s_Data->whiteTexture = Renderer::getWhiteTexture();
        s_Data->textureSlots[0] = s_Data->whiteTexture;

        PipelineSpecification pipelineSpec{
            .shader = s_Data->shader,
            .vertexBufferLayout = vertexBufferLayout,
            .depthTestEnable = true,
            .depthWriteEnable = false
        };
        s_Data->pipeline = Ref<VulkanPipeline>::create(pipelineSpec);

        s_Data->cameraUBA = UniformBufferArray::create(sizeof(CameraData));
        s_Data->descriptorManager = Ref<VulkanDescriptorManager>::create(s_Data->shader);
        s_Data->descriptorManager->setInput("u_Camera", s_Data->cameraUBA);

        for (uint32_t i = 0; i < Renderer2DData::maxTextureSlots; ++i)
        {
            s_Data->descriptorManager->setInput("u_Textures", s_Data->whiteTexture, i);
        }
        s_Data->descriptorManager->upload();

        s_Data->quadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_Data->quadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
        s_Data->quadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
        s_Data->quadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
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

        // Reset texture slots
        s_Data->textureSlotIndex = 1;

        for (size_t i = 1; i < s_Data->textureSlots.size(); ++i)
        {
            s_Data->textureSlots[i] = nullptr;
        }
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

            for (uint32_t i = 0; i < s_Data->textureSlots.size(); ++i)
            {
                if (s_Data->textureSlots[i])
                {
                    s_Data->descriptorManager->setInput("u_Textures", s_Data->textureSlots[i], i);
                }
                else
                {
                    s_Data->descriptorManager->setInput("u_Textures", s_Data->whiteTexture, i);
                }
            }

            Renderer::beginRenderPass(
                swapchain->getCurrentCommandBuffer(),
                swapchain->getRenderPass(),
                s_Data->pipeline,
                s_Data->descriptorManager->getDescriptorSets(Renderer::getCurrentFrameIndex())
            );
            //s_Data->descriptorManager->upload();

            Renderer::renderGeometry(
                swapchain->getCurrentCommandBuffer(),
                s_Data->pipeline,
                s_Data->descriptorManager,
                s_Data->quadVB,
                s_Data->quadIB,
                s_Data->quadIndexCount
            );

            Renderer::endRenderPass(swapchain->getCurrentCommandBuffer());

            s_Data->stats.drawCalls++;
        }

        Renderer::endFrame();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Quad rendering
    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Renderer2D::drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        drawQuad({ position.x, position.y, 0.0f }, size, color);
    }

    void Renderer2D::drawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        const float textureIndex = 0.0f;  // White texture
        const float tilingFactor = 1.0f;

        constexpr glm::vec2 texCoords[] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f }
        };

        constexpr size_t quadVertexCount = 4;
        for (size_t i = 0; i < quadVertexCount; ++i)
        {
            s_Data->quadVertexBufferPtr->position = transform * s_Data->quadVertexPositions[i];
            s_Data->quadVertexBufferPtr->color = color;
            s_Data->quadVertexBufferPtr->texCoord = texCoords[i];
            s_Data->quadVertexBufferPtr->texIndex = textureIndex;
            s_Data->quadVertexBufferPtr++;
        }

        s_Data->quadIndexCount += 6;

        s_Data->stats.quadCount++;
    }

    void Renderer2D::drawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        drawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
    }

    void Renderer2D::drawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        // Find texture index
        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data->textureSlotIndex; ++i)
        {
            if (*s_Data->textureSlots[i].raw() == *texture.raw())
            {
                textureIndex = (float)i;
                break;
            }
        }

        // If texture is not found, insert it into the texture slots
        if (textureIndex == 0.0f)
        {
            textureIndex = (float)s_Data->textureSlotIndex;
            s_Data->textureSlots[s_Data->textureSlotIndex] = texture;
            s_Data->textureSlotIndex++;
        }

        constexpr glm::vec2 texCoords[] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f }
        };
        constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

        constexpr size_t quadVertexCount = 4;
        for (size_t i = 0; i < quadVertexCount; ++i)
        {
            s_Data->quadVertexBufferPtr->position = transform * s_Data->quadVertexPositions[i];
            s_Data->quadVertexBufferPtr->color = { 1.0f, 1.0f, 1.0f, 1.0f };
            s_Data->quadVertexBufferPtr->texCoord = texCoords[i];
            s_Data->quadVertexBufferPtr->texIndex = textureIndex;
            s_Data->quadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data->quadVertexBufferPtr++;
        }

        s_Data->quadIndexCount += 6;

        s_Data->stats.quadCount++;
    }

    void Renderer2D::drawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float degrees, const glm::vec4& color)
    {
        drawRotatedQuad({ position.x, position.y, 0.0f }, size, degrees, color);
    }

    void Renderer2D::drawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float degrees, const glm::vec4& color)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(degrees), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        const float textureIndex = 0.0f;  // White texture
        const float tilingFactor = 1.0f;

        constexpr glm::vec2 texCoords[] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f }
        };

        constexpr size_t quadVertexCount = 4;
        for (size_t i = 0; i < quadVertexCount; ++i)
        {
            s_Data->quadVertexBufferPtr->position = transform * s_Data->quadVertexPositions[i];
            s_Data->quadVertexBufferPtr->color = color;
            s_Data->quadVertexBufferPtr->texCoord = texCoords[i];
            s_Data->quadVertexBufferPtr->texIndex = textureIndex;
            s_Data->quadVertexBufferPtr++;
        }

        s_Data->quadIndexCount += 6;

        s_Data->stats.quadCount++;
    }

    Renderer2D::Statistics Renderer2D::getStats() const
    {
        return s_Data->stats;
    }

    void Renderer2D::resetStats()
    {
        s_Data->stats.drawCalls = 0;
        s_Data->stats.quadCount = 0;
    }
}

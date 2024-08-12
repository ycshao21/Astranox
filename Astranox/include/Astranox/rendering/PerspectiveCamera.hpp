#pragma once
#include <glm/glm.hpp>
#include "Astranox/core/Timestep.hpp"

namespace Astranox
{
    class PerspectiveCamera: public RefCounted
    {
    public:
        PerspectiveCamera(float fov, float nearClip, float farClip);
        ~PerspectiveCamera();

        void onUpdate(Timestep ts);
        void onResize(uint32_t width, uint32_t height);

        const glm::mat4& getProjectionMatrix() const { return m_ProjectionMatrix; }
        const glm::mat4& getInverseProjectionMatrix() const { return m_InverseProjectionMatrix; }
        const glm::mat4& getViewMatrix() const { return m_ViewMatrix; }
        const glm::mat4& getInverseViewMatrix() const { return m_InverseViewMatrix; }

        const glm::vec3& getPosition() const { return m_Position; }
        const glm::vec3& getDirection() const { return m_Direction; }

    private:
        void updateProjectionMatrix();
        void updateViewMatrix();

    private:
        float m_Fov = 45.0f;
        float m_Near = 0.1f;
        float m_Far = 100.0f;

        glm::mat4 m_ProjectionMatrix{ 1.0f };
        glm::mat4 m_InverseProjectionMatrix{ 1.0f };

        glm::mat4 m_ViewMatrix{ 1.0f };
        glm::mat4 m_InverseViewMatrix{ 1.0f };

        glm::vec3 m_Position{ 0.0f, 0.0f, 3.0f };
        glm::vec3 m_Direction{ 0.0f, 0.0f, -1.0f };

        uint32_t m_ViewportWidth = 0;
        uint32_t m_ViewportHeight = 0;

        glm::vec2 m_LastMousePos{ 0.0f, 0.0f };

        float m_MoveSpeed = 4.0f;
        float m_RotationSpeed = 1000.0f;
    };
}

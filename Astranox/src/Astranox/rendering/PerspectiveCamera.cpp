#include "pch.hpp"
#include "Astranox/rendering/PerspectiveCamera.hpp"

#include "Astranox/core/Input.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Astranox
{
    PerspectiveCamera::PerspectiveCamera(float fov, float nearClip, float farClip)
        : m_Fov(fov), m_Near(nearClip), m_Far(farClip)
    {
    }

    PerspectiveCamera::~PerspectiveCamera()
    {
    }

    void PerspectiveCamera::onUpdate(Timestep ts)
    {
        glm::vec2 mousePos = Input::GetMousePosition();
        glm::vec2 delta = (mousePos - m_LastMousePos) * 0.002f;
        m_LastMousePos = mousePos;

        if (!Input::isMouseButtonPressed(MouseButton::Left))
        {
            Input::setCursorMode(CursorMode::Normal);
            return;
        }

        Input::setCursorMode(CursorMode::Locked);

        bool moved = false;

        constexpr glm::vec3 upDirection = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 rightDirection = glm::cross(m_Direction, upDirection);

        if (Input::isKeyPressed(Key::W))
        {
            m_Position += m_Direction * m_MoveSpeed * ts.getSeconds();
            moved = true;
        }
        else if (Input::isKeyPressed(Key::S))
        {
            m_Position -= m_Direction * m_MoveSpeed * ts.getSeconds();
            moved = true;
        }

        if (Input::isKeyPressed(Key::A))
        {
            m_Position -= rightDirection * m_MoveSpeed * ts.getSeconds();
            moved = true;
        }
        else if (Input::isKeyPressed(Key::D))
        {
            m_Position += rightDirection * m_MoveSpeed * ts.getSeconds();
            moved = true;
        }

        if (Input::isKeyPressed(Key::Space))
        {
            m_Position += upDirection * m_MoveSpeed * ts.getSeconds();
            moved = true;
        }
        else if (Input::isKeyPressed(Key::LeftShift))
        {
            m_Position -= upDirection * m_MoveSpeed * ts.getSeconds();
            moved = true;
        }

        if (delta.x != 0.0f || delta.y != 0.0f)
        {
            moved = true;

            float yawDelta = delta.x * m_RotationSpeed * ts.getSeconds();
            float pitchDelta = delta.y * m_RotationSpeed * ts.getSeconds();

            glm::quat q = glm::normalize(
                glm::cross(
                    glm::angleAxis(-pitchDelta, rightDirection),
                    glm::angleAxis(-yawDelta, upDirection)
                )
            );
            m_Direction = glm::normalize(q * m_Direction);
        }

        if (moved)
        {
            updateViewMatrix();
        }
    }

    void PerspectiveCamera::onResize(uint32_t width, uint32_t height)
    {
        if (m_ViewportWidth == width && m_ViewportHeight == height)
        {
            return;
        }
        
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        updateProjectionMatrix();
        updateViewMatrix();
    }

    void PerspectiveCamera::updateProjectionMatrix()
    {
        m_ProjectionMatrix = glm::perspectiveFov(glm::radians(m_Fov), (float)m_ViewportWidth, (float)m_ViewportHeight, m_Near, m_Far);
        m_InverseProjectionMatrix = glm::inverse(m_ProjectionMatrix);
    }

    void PerspectiveCamera::updateViewMatrix()
    {
        m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Direction, glm::vec3(0.0f, 1.0f, 0.0f));
        m_InverseViewMatrix = glm::inverse(m_ViewMatrix);
    }
}

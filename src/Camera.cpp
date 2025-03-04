#include "Camera.h"
#include <algorithm>
#include "../libs/glm/ext/matrix_transform.hpp"
#include "../libs/glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <Log.h>
#include <sstream>

#include "../libs/glm/gtx/dual_quaternion.hpp"

glm::vec3 up{0,1.0f,0};

Camera::Camera(const glm::vec3 pos, const float fov, const float width, const float height, const float near, const float far)
    :   m_View(glm::lookAt(m_Position, m_Position + m_Dir, up)),
        m_Projection(glm::perspective(fov/2.0f, width/height, near, far)),
        m_ViewProjection(m_Projection * m_View),
        m_Dir(0,0,1),
        m_Position(pos)
{
}

void Camera::translate(const glm::vec3& translation)
{
    m_Position += translation;
}

void Camera::move(const glm::vec3& translation)
{
    // left / right
    m_Position += -normalize(cross(m_Dir, up)) * translation.x;
    // up / down
    m_Position.y += translation.y;
    // front / back
    m_Position.z += translation.z * m_Dir.z;
}

void Camera::updateView()
{
    m_View = glm::lookAt(m_Position, m_Position + m_Dir, up);
    m_ViewProjection = m_Projection * m_View;
}

void Camera::rotate(const glm::dvec2 relPos)
{
    m_Yaw += relPos.x * m_Sensitivity;
    m_Pitch -= relPos.y * m_Sensitivity;
    m_Pitch = glm::clamp(m_Pitch, -89.0, 89.0);

    glm::vec3 front;
    front.x = cos(glm::radians(m_Pitch)) * cos(glm::radians(m_Yaw));
    front.y = sin(glm::radians(m_Pitch));
    front.z = cos(glm::radians(m_Pitch)) * sin(glm::radians(m_Yaw));
    m_Dir = normalize(front);
}

Camera::operator std::string() const
{
    std::ostringstream lStream;
    lStream << "Yaw: " << m_Yaw << " Pitch: " << m_Pitch << " Position: " << m_Position.x << " " << m_Position.y << " " << m_Position.z << " LookAt: " << m_Dir.x << " " << m_Dir.y << " " << m_Dir.z;
    return lStream.str();
}
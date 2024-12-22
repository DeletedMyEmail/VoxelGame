#include "../include/Camera.h"
#include <algorithm>
#include "../libs/glm/ext/matrix_transform.hpp"
#include "../libs/glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <sstream>

#include "../libs/glm/gtx/dual_quaternion.hpp"

Camera::Camera(const glm::vec3 pos, const float fov, const float height, const float width, const float near, const float far)
    :   m_LookAt(0,0,1),
        m_Position(pos),
        m_View(glm::mat4(1.0f)),
        m_Projection(glm::perspective(fov/2.0f, width/height, near, far))
{
    update();
}

void Camera::translate(const glm::vec3& translation) {
    m_View = glm::translate(m_View, translation);
    m_Position += translation;
}

void Camera::move(const glm::vec3& translation)
{
    moveFront(translation.z);
    moveSideways(translation.x);
    moveUp(translation.y);
}

void Camera::moveFront(const float val) {
    translate(val * m_LookAt);
}

void Camera::moveSideways(const float val) {
    translate(normalize(cross(m_LookAt, glm::vec3(0.0f, 1.0f, 0.0f))) * val);
}

void Camera::moveUp(const float val) {
    translate(val * glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::update() {
    m_View = glm::lookAt(m_Position, m_Position + m_LookAt, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::rotate(const double relX, const double relY) {
    m_Yaw += relX * m_Sensitivity;
    m_Pitch -= relY * m_Sensitivity;
    m_Pitch = std::clamp(m_Pitch, -89.0, 89.0);

    glm::vec3 front;
    front.x = cos(glm::radians(m_Pitch)) * cos(glm::radians(m_Yaw));
    front.y = sin(glm::radians(m_Pitch));
    front.z = cos(glm::radians(m_Pitch)) * sin(glm::radians(m_Yaw));
    m_LookAt = normalize(front);
}

Camera::operator std::string() const {
    std::ostringstream lStream;
    lStream << "Yaw: " << m_Yaw << " Pitch: " << m_Pitch << " Position: " << m_Position.x << " " << m_Position.y << " " << m_Position.z << " LookAt: " << m_LookAt.x << " " << m_LookAt.y << " " << m_LookAt.z;
    return lStream.str();
}
#include "Camera.h"
#include <algorithm>
#include "../libs/glm/ext/matrix_transform.hpp"
#include "../libs/glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <cstmlib/Log.h>
#include <sstream>
#include "../libs/glm/gtx/dual_quaternion.hpp"

Camera::Camera(const glm::vec3 pos, const float fov, const float width, const float height, const float near, const float far)
    :   projection(glm::perspective(glm::radians(fov), width/height, near, far)),
        position(pos),
        yaw(-90.0)
{
    updateLookDirection();
    updateView();
}

void Camera::move(const glm::vec3& translation)
{
    position += translation;
}

void Camera::updateView()
{
    updateLookDirection();
    const glm::vec3 worldUp{0.0f, 1.0f, 0.0f};
    const glm::vec3 right(glm::normalize(glm::cross(lookDir, worldUp)));
    const glm::vec3 up(glm::normalize(glm::cross(right, lookDir)));

    view = glm::lookAt(position, position + lookDir, up);
    viewProjection = projection * view;
}

void Camera::rotate(const glm::dvec2 relPos)
{
    yaw += relPos.x * sensitivity;
    pitch -= relPos.y * sensitivity;
    pitch = glm::clamp(pitch, -89.0, 89.0);

    updateLookDirection();
}

void Camera::updateLookDirection()
{
    glm::vec3 front;
    front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    front.y = sin(glm::radians(pitch));
    front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    lookDir = normalize(front);
}

Camera::operator std::string() const
{
    std::ostringstream lStream;
    lStream << "Yaw: " << yaw << " Pitch: " << pitch << " Position: " << position.x << " " << position.y << " " << position.z << " LookAt: " << lookDir.x << " " << lookDir.y << " " << lookDir.z;
    return lStream.str();
}
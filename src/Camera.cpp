#include "Camera.h"
#include <algorithm>
#include "../libs/glm/ext/matrix_transform.hpp"
#include "../libs/glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <cstmlib/Log.h>
#include <sstream>

#include "../libs/glm/gtx/dual_quaternion.hpp"

glm::vec3 up{0,1.0f,0};

Camera::Camera(const glm::vec3 pos, const float fov, const float width, const float height, const float near, const float far)
    :   view(glm::lookAt(position, position + lookDir, up)),
        projection(glm::perspective(fov/2.0f, width/height, near, far)),
        viewProjection(projection * view),
        lookDir(0,0,1),
        position(pos)
{
}

void Camera::translate(const glm::vec3& translation)
{
    position += translation;
}

void Camera::move(const glm::vec3& translation)
{
    const glm::vec3 right = normalize(cross(lookDir, up));
    // left / right
    position += right * translation.x;
    // up / down
    position.y += translation.y;
    // front / back
    position += lookDir * translation.z;
}

void Camera::updateView()
{
    view = glm::lookAt(position, position + lookDir, up);
    viewProjection = projection * view;
}

void Camera::rotate(const glm::dvec2 relPos)
{
    yaw += relPos.x * sensitivity;
    pitch -= relPos.y * sensitivity;
    pitch = glm::clamp(pitch, -89.0, 89.0);

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
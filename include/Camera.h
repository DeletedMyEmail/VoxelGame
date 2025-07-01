#pragma once

#include <string>
#include "../libs/glm/glm.hpp"

struct Camera
{
    Camera(glm::vec3 pos, float fov, float width, float height, float near, float far);

    void updateView();
    void move(const glm::vec3& translation);
    void translate(const glm::vec3& translation);
    void rotate(glm::dvec2 relPos);

    operator std::string() const;

    glm::mat4 view, projection, viewProjection;
    glm::vec3 lookDir, position;
    double yaw = 0, pitch = 0;
    float sensitivity = 0.1f;

private:
    void updateLookDirection();
};
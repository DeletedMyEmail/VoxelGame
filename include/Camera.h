#pragma once

#include <string>
#include "../libs/glm/glm.hpp"

class Camera {
public:
    Camera(glm::vec3 pos, float fov, float height, float width, float near, float far);

    void updateView();
    void move(const glm::vec3& translation);
    void translate(const glm::vec3& translation);
    void rotate(glm::dvec2 relPos);

    operator std::string() const;

    float getSensitive() const { return m_Sensitivity; }
    const glm::vec3& getPosition() const { return m_Position; }
    const glm::mat4& getViewProjection() const { return m_ViewProjection; }
    const glm::vec3& getLookDir() const { return m_Dir; }
private:
    glm::mat4 m_View, m_Projection, m_ViewProjection;
    glm::vec3 m_Dir, m_Position;
    double m_Yaw = 0, m_Pitch = 0;
    float m_Sensitivity = 0.1f;
};
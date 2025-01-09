#pragma once

#include <string>
#include "../libs/glm/glm.hpp"

class Camera {
public:
    Camera(glm::vec3 pos, float fov, float height, float width, float near, float far);

    void update(glm::dvec2 mousePos);
    void move(const glm::vec3& translation);
    void rotate(double relX, double relY);
    void increaseSpeed(const float speed) { m_Speed = glm::max(m_Speed + speed, 0.0f); }
    operator std::string() const;

    float getSensitive() const { return m_Sensitivity; }
    glm::vec3 getPosition() const { return m_Position; }
    float getSpeed() const { return m_Speed; }
    const glm::mat4& getViewProjection() const { return m_ViewProjection; }
    glm::vec3 getLookAt() const { return m_Dir; }
private:
    void processCursorMovement(glm::dvec2 mousePos);
    void translate(const glm::vec3& translation);
    void moveFront(float val);
    void moveSideways(float val);
    void moveUp(float val);
private:
    float m_Sensitivity = 0.1f, m_Speed = 5.0f;
    double m_Yaw = 0, m_Pitch = 0;
    glm::vec3 m_Dir, m_Position;
    glm::mat4 m_View, m_Projection, m_ViewProjection{};
};
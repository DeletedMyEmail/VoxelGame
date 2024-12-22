#pragma once

#include <string>

#include "../libs/glm/glm.hpp"

class Camera {
public:
    Camera(glm::vec3 pos, float fov, float height, float width, float near, float far);

    void translate(const glm::vec3& translation);
    void move(const glm::vec3& translation);
    void update();
    void rotate(double relX, double relY);
    operator std::string() const;

    void setSensitive(const float sensitivity) { m_Sensitivity = sensitivity; }
    void increaseSpeed(const float speed) { m_Speed = glm::max(m_Speed + speed, 0.0f); }
    float getSensitive() const { return m_Sensitivity; }
    float getSpeed() const { return m_Speed; }
    const glm::mat4& getView() const { return m_View; }
    const glm::mat4& getProjection() const { return m_Projection; }
private:
    void moveFront(float val);
    void moveSideways(float val);
    void moveUp(float val);
private:
    float m_Sensitivity = 0.1f, m_Speed = 5.0f;
    double m_Yaw = 0, m_Pitch = 0;
    glm::vec3 m_LookAt, m_Position;
    glm::mat4 m_View, m_Projection;
};
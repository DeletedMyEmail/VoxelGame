#pragma once

#include <string>
#include "../libs/glm/glm.hpp"

class Camera {
public:
    Camera(glm::vec3 pos, float fov, float height, float width, float near, float far);

    void update(glm::dvec2 mousePos);
    void move(const glm::vec3& translation);
    void translate(const glm::vec3& translation);
    void rotate(double relX, double relY);
    operator std::string() const;

    float getSensitive() const { return m_Sensitivity; }
    const glm::vec3& getPosition() const { return m_Position; }
    const glm::mat4& getViewProjection() const { return m_ViewProjection; }
    const glm::vec3& getLookDir() const { return m_Dir; }
private:
    void processCursorMovement(glm::dvec2 mousePos);
    void moveFront(float val);
    void moveSideways(float val);
    void moveUp(float val);
private:
    glm::mat4 m_View, m_Projection, m_ViewProjection;
    glm::vec3 m_Dir, m_Position;
    double m_Yaw = 0, m_Pitch = 0;
    float m_Sensitivity = 0.1f;
};
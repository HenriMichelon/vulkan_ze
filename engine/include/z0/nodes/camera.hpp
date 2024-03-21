#pragma once

#include "z0/nodes/node.hpp"

namespace z0 {
    class Camera: public Node {
    public:
        explicit Camera(const std::string nodeName = "Camera");

        void setOrthographicProjection(float left, float right,
                                       float top, float bottom,
                                       float near, float far);
        void setPerspectiveProjection(float fov,
                                      float near, float far);

        void setPosition(glm::vec3 position);
        void setRotation(glm::vec3 orientation);
        void setRotationX(float angle);
        void setRotationY(float angle);
        void setRotationZ(float angle);

        const glm::mat4& getProjection();
        const glm::mat4& getView() const { return viewMatrix; }
        const glm::mat4& getInverseView() const { return inverseViewMatrix; }
        const glm::vec3 getPosition() const { return glm::vec3(inverseViewMatrix[3]); }

    private:
        float fov;
        float nearDistance;
        float farDistance;
        bool usePerspectiveProjection{false};
        glm::mat4 projectionMatrix{1.0f};
        glm::mat4 viewMatrix{1.0f};
        glm::mat4 inverseViewMatrix{1.0f};

        const glm::vec3 direction{0.0f, 0.0f, 1.0f };
        const glm::vec3 up{0.0f, -1.0f, 0.0f};

        void setViewDirection();
        //void setViewYXZ();
        //void setViewTarget(glm::vec3 target);

    };
}
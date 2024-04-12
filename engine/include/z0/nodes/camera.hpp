#pragma once

#include "z0/nodes/node.hpp"

namespace z0 {
    class Camera: public Node {
    public:
        explicit Camera(const std::string nodeName = "Camera");
        virtual ~Camera() {};

        void setOrthographicProjection(float left, float right,
                                       float top, float bottom,
                                       float near, float far);
        void setPerspectiveProjection(float fov,
                                      float near, float far);

        const glm::mat4& getProjection();
        const glm::mat4& getView() const { return viewMatrix; }

        void updateTransform(const glm::mat4& parentMatrix);
        void updateTransform();

        void setCurrent();
        bool isCurrent();

        void _onEnterScene() override;
        void _onExitScene() override;

    private:
        float fov{75.0};
        float nearDistance{0.1f};
        float farDistance{100.1f};
        bool usePerspectiveProjection{false};
        glm::mat4 projectionMatrix{1.0f};
        glm::mat4 viewMatrix{1.0f};

        const glm::vec3 direction{0.0f, 0.0f, 1.0f };

        void setViewDirection();

    };
}
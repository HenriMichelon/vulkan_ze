#pragma once

#include "node.hpp"

namespace z0 {

    class Light: public Node {
    public:
        explicit Light(const std::string nodeName = ""): Node{nodeName} {}
        Light(glm::vec4 color, float specular = 1.0f, const std::string nodeName = ""):
            Node{nodeName}, colorAndIntensity{color}, specularIntensity{specular}  {}

        glm::vec4& getColorAndIntensity() { return colorAndIntensity; }
        void setColorAndIntensity(glm::vec4 color) { colorAndIntensity = color; }
        float getSpecularIntensity() const { return specularIntensity; }
        void setSpecularIntensity(float specular) { specularIntensity = specular; }

    private:
        glm::vec4 colorAndIntensity{1.0f, 1.0f, 1.0f, 1.0f}; // RGB + Intensity
        float specularIntensity{1.0f};
    };

}
#pragma once

#include "node.hpp"

namespace z0 {

    class Light: public Node {
    public:
        explicit Light(const std::string nodeName = "Light"): Node{nodeName} {}
        explicit Light(glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f},
              float specular = 1.0f,
              const std::string nodeName = "Light"):
            Node{nodeName}, colorAndIntensity{color}, specularIntensity{specular}  {}
        virtual ~Light() {};

        glm::vec4& getColorAndIntensity() { return colorAndIntensity; }
        void setColorAndIntensity(glm::vec4 color) { colorAndIntensity = color; }
        float getSpecularIntensity() const { return specularIntensity; }
        void setSpecularIntensity(float specular) { specularIntensity = specular; }
        bool getCastShadows() const { return castShadows; }
        void setCastShadow(bool cast) { castShadows = cast; }

    private:
        glm::vec4 colorAndIntensity {1.0f, 1.0f, 1.0f, 1.0f};
        float specularIntensity {1.0f};
        bool castShadows {false};
    };

}
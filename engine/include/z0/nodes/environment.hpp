#pragma once

#include "z0/nodes/node.hpp"

namespace z0 {

    class Environment : public Node {
    public:
        explicit Environment(const std::string nodeName = ""): Node{nodeName} {}

        glm::vec4& getAmbientColorAndIntensity() { return ambientColorIntensity; }
        void setAmbientColorAndIntensity(glm::vec4 color) { ambientColorIntensity = color; }

    private:
        glm::vec4 ambientColorIntensity{0.0f, 0.0f, 0.0f, 0.0f};
    };

}
#pragma once

#include "z0/nodes/Node.hpp"

namespace z0 {

    class Loader {
    public:
        static std::shared_ptr<Node> loadModelFromFile(const std::filesystem::path& filepath, bool forceBackFaceCulling = false);
    };
}
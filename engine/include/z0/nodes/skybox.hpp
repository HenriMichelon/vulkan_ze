#pragma once

#include "node.hpp"
#include "z0/cubemap.hpp"

namespace z0 {

    class Skybox: public Node {
    public:
        explicit Skybox(const std::filesystem::path& filename, const std::string& fileext, const std::string nodeName = "Skybox");

        std::shared_ptr<Cubemap>& getCubemap() { return cubemap; }

    private:
        std::shared_ptr<Cubemap> cubemap;
    };

}
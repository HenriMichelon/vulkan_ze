#pragma once

#include "node.hpp"
#include "z0/resources/cubemap.hpp"

namespace z0 {

    class Skybox: public Node {
    public:
        explicit Skybox(const std::filesystem::path& filename, const std::string& fileext, const std::string nodeName = "Skybox");
        virtual ~Skybox() {};

        std::shared_ptr<Cubemap>& getCubemap() { return cubemap; }

        void _onEnterTree() override;

    private:
        std::shared_ptr<Cubemap> cubemap;
    };

}
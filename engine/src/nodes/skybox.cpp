#include "z0/nodes/skybox.hpp"

namespace z0 {

    Skybox::Skybox(const std::filesystem::path& filename, const std::string& fileext, const std::string nodeName):
        Node{nodeName}{
        cubemap = std::make_shared<Cubemap>(filename, fileext);
    }

}
#include "z0/application.hpp"
#include "z0/viewport.hpp"
#include "z0/nodes/skybox.hpp"

namespace z0 {

    Skybox::Skybox(const std::filesystem::path& filename, const std::string& fileext, const std::string nodeName):
        Node{nodeName}{
        cubemap = std::make_shared<Cubemap>(filename, fileext);
    }

    void Skybox::_onEnterScene() {
        Application::getViewport()._setSkyBox(*this);
        Node::_onEnterScene();
    }

    void Skybox::_onExitScene() {
        Node::_onExitScene();
    }

}
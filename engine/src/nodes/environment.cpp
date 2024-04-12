#include "z0/nodes/environment.hpp"
#include "z0/application.hpp"
#include "z0/viewport.hpp"

namespace z0 {

    void Environment::_onEnterScene() {
        Application::getViewport().setEnvironment(this);
        Node::_onEnterScene();
    }

}
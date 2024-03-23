#include "z0/helpers/window_helper.hpp"
#include "z0/input.hpp"
#include "z0/application.hpp"

namespace z0 {

#ifdef GLFW_VERSION_MAJOR
    inline static GLFWwindow* getWindowHandle() { return Application::getViewport()._getWindowHelper().getWindowHandle(); }
#endif

    std::list<std::shared_ptr<InputEvent>> Input::inputQueue;

    void Input::injectInputEvent(std::shared_ptr<InputEvent> event) {
        inputQueue.push_back(event);
    }

    std::shared_ptr<InputEvent> Input::consumeInputEvent() {
        auto event = inputQueue.front();
        inputQueue.pop_front();
        return event;
    }

#ifdef GLFW_VERSION_MAJOR
    bool Input::isKeyPressed(Key key) {
        return glfwGetKey(getWindowHandle(), key) == GLFW_PRESS;
    }

    void Input::setMouseMode(MouseMode mode) {
        int value = 0;
        switch (mode) {
            case MOUSE_MODE_VISIBLE:
                value = GLFW_CURSOR_NORMAL;
                break;
            case MOUSE_MODE_VISIBLE_CAPTURED:
                value = GLFW_CURSOR_CAPTURED;
                break;
            case MOUSE_MODE_HIDDEN:
                value = GLFW_CURSOR_HIDDEN;
                break;
            case MOUSE_MODE_HIDDEN_CAPTURED:
                value = GLFW_CURSOR_DISABLED;
                break;
        }
        glfwSetInputMode(getWindowHandle(), GLFW_CURSOR, value);
    }
#endif

}
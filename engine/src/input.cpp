#include "z0/helpers/window_helper.hpp"
#include "z0/input.hpp"
#include "z0/application.hpp"

namespace z0 {

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
    inline static GLFWwindow* getWindowHandle() { return Application::getViewport()._getWindowHelper().getWindowHandle(); }

    bool Input::isMouseButtonPressed(MouseButton mouseButton) {
        return glfwGetMouseButton(getWindowHandle(), mouseButton) == GLFW_PRESS;
    }

    glm::vec2 Input::getGamepadVector(int index, GamepadAxisJoystick axisJoystick) {
        GLFWgamepadstate state;
        if (glfwGetGamepadState(index, &state)) {
            glm::vec2 vector{
                    state.axes[axisJoystick == GAMEPAD_AXIS_LEFT ? GAMEPAD_AXIS_LEFT_X : GAMEPAD_AXIS_RIGHT_X],
                    state.axes[axisJoystick == GAMEPAD_AXIS_LEFT ? GAMEPAD_AXIS_LEFT_Y : GAMEPAD_AXIS_RIGHT_Y],
                };
            float length = vector.length();
            if (length > 1.0f) {
                return vector / length;
            } else {
                return vector;
            }
        }
        return glm::vec2{0.0, 0.0};
    }

    bool Input::isGamepadButtonPressed(int index, GamepadButton gamepadButton) {
        GLFWgamepadstate state;
        if (glfwGetGamepadState(index, &state)) {
            return state.buttons[gamepadButton] == GLFW_PRESS;
        }
        return false;
    }

    float Input::getGamepadAxisValue(int index, GamepadAxis gamepadAxis) {
        GLFWgamepadstate state;
        if (glfwGetGamepadState(index, &state)) {
            auto value = state.axes[gamepadAxis];
            return (value > 0.1) || (value < -0.1) ? value : 0.0;
        }
        return false;
    }

    int Input::getConnectedJoypads() {
        int count = 0;
        for (int i = 0; i < GLFW_JOYSTICK_LAST; i++) {
            if (glfwJoystickPresent(i)) count += 1;
        }
        return count;
    }

    bool Input::isGamepad(int index) {
        return glfwJoystickIsGamepad(index);
    }

    std::string Input::getGamepadName(int index) {
        return glfwGetGamepadName(index);
    }

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
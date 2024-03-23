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
    const float gamepadDeadZone = 0.1f;

    bool Input::isMouseButtonPressed(MouseButton mouseButton) {
        return glfwGetMouseButton(getWindowHandle(), mouseButton) == GLFW_PRESS;
    }

    glm::vec2 Input::getGamepadVector(int index, GamepadAxisJoystick axisJoystick) {
        GLFWgamepadstate state;
        if (glfwGetGamepadState(index, &state)) {
            auto x = state.axes[axisJoystick == GAMEPAD_AXIS_LEFT ? GAMEPAD_AXIS_LEFT_X : GAMEPAD_AXIS_RIGHT_X];
            auto y = state.axes[axisJoystick == GAMEPAD_AXIS_LEFT ? GAMEPAD_AXIS_LEFT_Y : GAMEPAD_AXIS_RIGHT_Y];
            glm::vec2 vector{
                    (x > gamepadDeadZone) || (x < -gamepadDeadZone) ? x : 0.0,
                    (y > gamepadDeadZone) || (y < -gamepadDeadZone) ? y : 0.0,
                };
            float length = vector.length();
            return (length > 1.0f) ? vector / length : vector;
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
            return (value > gamepadDeadZone) || (value < -gamepadDeadZone) ? value : 0.0;
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

    glm::vec2 Input::getKeyboardVector(Key negX, Key posX, Key negY, Key posY) {
        auto hdl = getWindowHandle();
        auto x = glfwGetKey(hdl, negX) == GLFW_PRESS ? -1 : glfwGetKey(hdl, posX) == GLFW_PRESS ? 1 : 0;
        auto y = glfwGetKey(hdl, negY) == GLFW_PRESS ? -1 : glfwGetKey(hdl, posY) == GLFW_PRESS ? 1 : 0;
        glm::vec2 vector{ x, y };
        float length = vector.length();
        return (length > 1.0f) ? vector / length : vector;
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
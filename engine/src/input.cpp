#include "z0/input.hpp"
#include "z0/application.hpp"

namespace z0 {

    bool Input::isKeyPressed(Key key) {
        return Application::getViewport().isKeyPressed(key);
    }

    void Input::setMouseMode (MouseMode mode) {
        Application::getViewport().setMouseMode(mode);
    }

    InputEventKey::InputEventKey(Key _key, bool _pressed, bool _repeat, int _modifiers):
            InputEvent{INPUT_EVENT_KEY},
            keycode{_key}, repeat{_repeat}, pressed{_pressed}, modifiers{_modifiers} {}

    InputEventMouseMotion::InputEventMouseMotion(float posX, float posY, float rX, float rY):
            InputEvent{INPUT_EVENT_MOUSE_MOTION},
            x{posX}, y{posY}, relativeX{rX}, relativeY{rY} {}

}
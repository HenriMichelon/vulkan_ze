#include "z0/input.hpp"
#include "z0/application.hpp"

namespace z0 {

    bool Input::isKeyPressed(Key key) {
        return Application::getViewport().isKeyPressed(key);
    }

    InputEventKey::InputEventKey(Key _key, bool _pressed, bool _repeat, int _modifiers):
            InputEvent{INPUT_EVENT_KEY},
            keycode{_key}, repeat{_repeat}, pressed{_pressed}, modifiers{_modifiers} {}

}
#include "z0/input_event.hpp"

namespace z0 {

    InputEventKey::InputEventKey(Key _key, bool _pressed, bool _repeat, int _modifiers):
            InputEvent{INPUT_EVENT_KEY},
            keycode{_key}, repeat{_repeat}, pressed{_pressed}, modifiers{_modifiers} {}

}
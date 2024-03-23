#pragma once

#include "z0/application.hpp"
#include "z0/input_event.hpp"

#include <map>

namespace z0 {

    class Input {
    public:
        static bool isKeyPressed(Key key);
        static glm::vec2 getKeyboardVector(Key negX, Key posX, Key negY, Key posY);

        static bool isMouseButtonPressed(MouseButton mouseButton);
        static void setMouseMode(MouseMode mode);

        static int getConnectedJoypads();
        static bool isGamepad(int index);
        static std::string getGamepadName(int index);
        static bool isGamepadButtonPressed(int index, GamepadButton gamepadButton);
        static float getGamepadAxisValue(int index, GamepadAxis gamepadAxis);
        static glm::vec2 getGamepadVector(int index, GamepadAxisJoystick axisJoystick);

        inline static bool haveInputEvent() { return !inputQueue.empty(); }
        static std::shared_ptr<InputEvent> consumeInputEvent();
        static void injectInputEvent(std::shared_ptr<InputEvent> event);

    public:
        static std::list<std::shared_ptr<InputEvent>> inputQueue;
    };


}
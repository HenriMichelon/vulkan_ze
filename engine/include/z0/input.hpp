#pragma once

#include "z0/application.hpp"
#include "z0/input_event.hpp"

namespace z0 {

    class Input {
    public:
        static bool isKeyPressed(Key key);
        static void setMouseMode(MouseMode mode);

        inline static bool haveInputEvent() { return !inputQueue.empty(); }
        static std::shared_ptr<InputEvent> consumeInputEvent();
        static void injectInputEvent(std::shared_ptr<InputEvent> event);

    public:
        static std::list<std::shared_ptr<InputEvent>> inputQueue;
    };

}
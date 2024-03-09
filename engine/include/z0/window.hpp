#pragma once

namespace z0 {

    enum WindowMode {
        WINDOW_MODE_WINDOWED            = 0,
        WINDOW_MODE_WINDOWED_MAXIMIZED  = 1,
        WINDOW_MODE_WINDOWED_FULLSCREEN = 2,
        WINDOW_MODE_FULLSCREEN          = 3,
        //WINDOW_MODE_FULLSCREEN_EXCLUSIVE= 4,
    };

    const int WINDOW_CLEAR_COLOR[] { 0, 0, 0 };
}
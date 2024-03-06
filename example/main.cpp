#include <iostream>
#include "z0/application.hpp"

class ExampleApp : z0::Application {
public:
    ExampleApp(): z0::Application{z0::WINDOW_MODE_WINDOWED_MAXIMIZED, 1024, 768,
                                  "Example App", "..", z0::MSAA_AUTO} {

    };
};

int main() {
    ExampleApp{};
    return 0;
}

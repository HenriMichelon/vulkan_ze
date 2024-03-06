#include <iostream>
#include "z0/application.hpp"
#include "z0/object.hpp"

class ExampleApp : z0::Application {
public:
    ExampleApp(): z0::Application{z0::WINDOW_MODE_WINDOWED, 1024, 768,
                                  "Example App", "..", z0::MSAA_DISABLED} {

    };
};

int main() {
    ExampleApp{};
    return 0;
}

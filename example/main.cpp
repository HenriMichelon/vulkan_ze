#include "z0/application.hpp"

class ExampleApp : z0::Application {
public:
    ExampleApp(const z0::ApplicationConfig& cfg): z0::Application{cfg} {

    };
};

int main() {
    z0::ApplicationConfig applicationConfig {
        .appName = "Example App",
        .appDir = "..",
        .windowMode = z0::WINDOW_MODE_WINDOWED,
        .windowWidth = 1024,
        .windowHeight = 768,
        .msaa = z0::MSAA_AUTO
    };
    ExampleApp{applicationConfig};
    return 0;
}

#include "z0/application.hpp"

class ExampleApp : z0::Application {
public:
    ExampleApp(): z0::Application{800, 600, "Example App"} {};
};

int main() {
    ExampleApp{};
    return 0;
}

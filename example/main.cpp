#include "z0/application.hpp"

class ExampleApp : z0::Application {
public:
    ExampleApp(): z0::Application{1024, 768, "Example App"} {};
};

int main() {
    ExampleApp{};
    return 0;
}

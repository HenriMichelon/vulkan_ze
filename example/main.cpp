#include <iostream>
#include "z0/application.hpp"
#include "z0/object.hpp"

class ExampleApp : z0::Application {
public:
    ExampleApp(): z0::Application{1024, 768, "Example App", ".."} {

    };
};

int main() {
    ExampleApp{};
    return 0;
}

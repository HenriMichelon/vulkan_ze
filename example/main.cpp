#include "z0/mainloop.hpp"
#include "z0/scene.hpp"
#include "z0/loader.hpp"

class RootNode: public z0::Node {
public:
    RootNode(): z0::Node("Main") {}

    void onReady() override {
        std::shared_ptr<Node> model = z0::Loader::loadModelFromFile("models/tc.glb", true);
        addChild(model);
        printTree(std::cout);
        model->scale(glm::vec3{ 5 });
        float angle = glm::radians(180.0);
        //model->rotate({ angle, .0, .0f });
    }

    void onProcess(float delta) override {
        float angle = delta * glm::radians(90.0f) / 2;
        //rotate({ .0, angle, angle });
    }
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
    z0::MainLoop app{applicationConfig};
    app.start(std::make_shared<z0::Scene>(std::make_shared<RootNode>()));
    return 0;
}

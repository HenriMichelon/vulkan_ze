#include "z0/mainloop.hpp"
#include "z0/scene.hpp"
#include "z0/loader.hpp"

class RootNode: public z0::Node {
public:
    RootNode(): z0::Node("Main") {}

    void onReady() override {
        std::shared_ptr<Node> model = z0::Loader::loadModelFromFile("models/free_1972_datsun_240k_gt.glb", false);
        addChild(model);
        printTree(std::cout);
        model->scale(glm::vec3{ 0.2 });
        model->rotate_degree({ 190.0, 45.0, -10.0 });
        model->translate({.5, 0., 0.0});
    }

    void onProcess(float delta) override {
        float angle = delta * glm::radians(90.0f) / 5;
        rotate({ .0, angle, .0 });
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

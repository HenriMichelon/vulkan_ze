#include "z0/mainloop.hpp"
#include "z0/scene.hpp"
#include "z0/loader.hpp"

class RootNode: public z0::Node {
public:
    void onReady() override {
        std::shared_ptr<Node> model = z0::Loader::loadModelFromFile("models/cube.glb", true);
        addChild(model);
    }

    void onProcess(float delta) override {
        float angle = delta * glm::radians(90.0f) / 2;
        transform.rotation = { angle, -angle, angle };
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

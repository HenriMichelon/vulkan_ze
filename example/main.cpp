#include "z0/mainloop.hpp"
#include "z0/scene.hpp"
#include "z0/loader.hpp"
#include "z0/nodes/mesh_instance.hpp"

class RootNode: public z0::Node {
public:
    RootNode(): z0::Node("Main") {}

    void onReady() override {
        //std::shared_ptr<Node> model = z0::Loader::loadModelFromFile("models/free_1972_datsun_240k_gt.glb", false);
        std::shared_ptr<z0::Node> model = z0::Loader::loadModelFromFile("models/cube2.glb", true);

        addChild(model);
        printTree(std::cout);
        model->scale(glm::vec3{ 1.2 });
        model->rotate_degree({ 180.0-16.0,  -25.0, 10.0 });
    }

    void onProcess(float delta) override {
        float angle = delta * glm::radians(90.0f) / 2;
        //rotate({ .0, angle, .0 });
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

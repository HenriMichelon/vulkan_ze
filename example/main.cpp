#include "z0/mainloop.hpp"
#include "z0/scene.hpp"
#include "z0/loader.hpp"
#include "z0/nodes/directional_light.hpp"
#include "z0/nodes/camera.hpp"
#include "z0/nodes/environment.hpp"
#include "z0/nodes/omni_light.hpp"
#include "z0/nodes/spot_light.hpp"

class RootNode: public z0::Node {
public:
    RootNode(): z0::Node("Main") {}

    void onReady() override {
        z0::Environment environment{};
        environment.setAmbientColorAndIntensity({1.0f, 1.0f, 1.0f, 0.1f});
        addChild(environment);

        z0::Camera camera{};
        camera.setPosition({-0.0f, -15.0f, -10.000001f });
        //camera.setViewDirection({.0, .0, 1.0}, {0.0, -1.0, 0.0 });
        camera.setViewTarget({ 0.0f, -0.0f, 0.0f});
        addChild(camera);

        z0::DirectionalLight directionalLight{glm::vec3{0.0f, .5f, 0.5f}};
        directionalLight.setColorAndIntensity({1.0f, 1.0f, 1.0f, 1.0f});
        addChild(directionalLight);

        /*z0::OmniLight omniLight1 {0.07, 0.017};
        omniLight1.setPosition({0.0f, -6.0f, 0.f});
        addChild(omniLight1);*/

        z0::SpotLight spotLight1{{0.0, 1.0f, -1.0f},
                                 45.0, 55.0,
                                 0.027, 0.0028};
        spotLight1.setSpecularIntensity(10.0);
        spotLight1.setPosition({0.0f, -10.0f, 10.f});
        spotLight1.setColorAndIntensity({1.0f, 1.0f, 1.0f, 2.0f});
        //addChild(spotLight1);

        //model1 = z0::Loader::loadModelFromFile("models/survival_guitar_backpack.glb", false);
        model1 = z0::Loader::loadModelFromFile("models/light.glb", false);
        model1->setPosition(spotLight1.getPosition());
        //model1->setScale(glm::vec3{0.01});
        //model1->setRotationDegrees({-180.0, -30.0, 0.0});
        addChild(model1);

        floor = z0::Loader::loadModelFromFile("models/floor.glb", false);
        //floor->setScale(glm::vec3{1.0});
        floor->setRotationDegrees({-180.0, 0.0, 0.0});
        addChild(floor);

        //printTree(std::cout);
    }

    void onProcess(float delta) override {
        float angle = delta * glm::radians(90.0f) / 2;
    }

private:
    std::shared_ptr<z0::Node> model1;
    std::shared_ptr<z0::Node> floor;
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

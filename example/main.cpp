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
        environment.setAmbientColorAndIntensity({1.0f, 1.0f, 1.0f, 0.02f});
        addChild(environment);

        z0::DirectionalLight directionalLight{glm::vec3{0.0f, .5f, 0.5f}};
        directionalLight.setColorAndIntensity({1.0f, 1.0f, 1.0f, 0.2f});
        //addChild(directionalLight);

        z0::SpotLight spotLight1{{-0.75, .5, 0.1},
                                 45.0, 55.0,
                                 0.027, 0.0028};
        spotLight1.setPosition({3.0, -6.0, -2.1});
        spotLight1.setColorAndIntensity({1.0f, 1.0f, 1.0f, 4.0f});
        addChild(spotLight1);
        light = z0::Loader::loadModelFromFile("models/light.glb", false);
        light->setPosition(spotLight1.getPosition());
        addChild(light);

        model1 = z0::Loader::loadModelFromFile("models/cube2.glb", false);
        //model1->setRotationDegrees({0.0, 30.0, 0.0});

        model2 = model1->duplicate();
        addChild(model2);
        addChild(model1);

        model1->setScale(glm::vec3{.5});
        model1->setPosition({2.0, 0.0, 0.0});
        model2->setPosition({0.0, -1.0, 0.0});

        floor = z0::Loader::loadModelFromFile("models/floor.glb", false);
        floor->setPosition({0.0, 2.0, 0.0});
        addChild(floor);

        z0::Camera camera{};
        camera.setPosition({ -2.0f, -1.0f, -10.0f});
        camera.setViewTarget({ 0.0f, 0.0f, 0.0f});
        //camera.setPosition(spotLight1.getPosition());
        //camera.setViewDirection(spotLight1.getDirection());
        addChild(camera);

        printTree(std::cout);
    }

    void onProcess(float delta) override {
        float angle = delta * glm::radians(90.0f) / 2;
        //model1->setRotationY(angle);
    }

private:
    std::shared_ptr<z0::Node> model1;
    std::shared_ptr<z0::Node> model2;
    std::shared_ptr<z0::Node> model3;
    std::shared_ptr<z0::Node> light;
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

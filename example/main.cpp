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

        /*z0::Camera camera{};
        camera.setPosition({-0.0f, -0.0f, -10.000001f });
        camera.setViewTarget({ 0.0f, 0.0f, 0.0f});
        addChild(camera);
        model1 = z0::Loader::loadModelFromFile("models/survival_guitar_backpack.glb", false);
        model1->setScale(glm::vec3{0.01});
        model1->setRotationDegrees({0.0, 0.0, 0.0});
        addChild(model1);*/

        z0::Camera camera{};
        camera.setPosition({-0.0f, -8.0f, -10.000001f });
        //camera.setViewDirection({.0, .0, 1.0}, {0.0, -1.0, 0.0 });
        camera.setViewTarget({ 0.0f, -4.0f, 0.0f});
        addChild(camera);

        z0::DirectionalLight directionalLight{glm::vec3{0.0f, .5f, 0.5f}};
        directionalLight.setColorAndIntensity({1.0f, 1.0f, 1.0f, 0.2f});
        //addChild(directionalLight);

        z0::SpotLight spotLight1{{.0, 2.0f, 2.0f},
                                 45.0, 55.0,
                                 0.027, 0.0028};
        //spotLight1.setSpecularIntensity(2.0);
        spotLight1.setPosition({-4.0f, -6.0f, -4.f});
        spotLight1.setColorAndIntensity({1.0f, 1.0f, 1.0f, 4.0f});
        addChild(spotLight1);
        light = z0::Loader::loadModelFromFile("models/light.glb", false);
        light->setPosition(spotLight1.getPosition());
        addChild(light);

        model1 = z0::Loader::loadModelFromFile("models/survival_guitar_backpack.glb", false);
        model1->setPosition({-4.0, -2.8f, -2.0f});
        model1->setScale(glm::vec3{0.01});
        model1->setRotationDegrees({0.0, -30.0, 0.0});
        addChild(model1);

        model2 = z0::Loader::loadModelFromFile("models/cube2.glb", false);
        model2->setPosition({2.0, -2.0f, 5.0f});
        model2->setRotationDegrees({0.0, -30.0, 0.0});
        addChild(model2);

        model3 = model2->duplicate();
        model3->setPosition({8.0, -4, 0.0f});
        model3->setRotationDegrees({0.0, 30.0, 0.0});
        addChild(model3);

        floor = z0::Loader::loadModelFromFile("models/floor.glb", false);
        //floor->setScale(glm::vec3{1.0});
        floor->setRotationDegrees({0.0, 0.0, 0.0});
        //addChild(floor);

        //printTree(std::cout);
    }

    void onProcess(float delta) override {
        float angle = delta * glm::radians(90.0f) / 2;
        model1->setRotationY(angle);
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
        .msaa = z0::MSAA_DISABLED
    };
    z0::MainLoop app{applicationConfig};
    app.start(std::make_shared<z0::Scene>(std::make_shared<RootNode>()));
    return 0;
}

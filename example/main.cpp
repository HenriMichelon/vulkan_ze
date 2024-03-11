#include "z0/mainloop.hpp"
#include "z0/scene.hpp"
#include "z0/loader.hpp"
#include "z0/nodes/directional_light.hpp"
#include "z0/nodes/camera.hpp"
#include "z0/nodes/environment.hpp"

class RootNode: public z0::Node {
public:
    RootNode(): z0::Node("Main") {}

    void onReady() override {
        z0::Environment environment{"Exterior"};
        environment.setAmbientColorAndIntensity({1.0f, 1.0f, 1.0f, 0.1f});
        addChild(environment);

        z0::Camera camera{"Camera1"};
        camera.setPosition({-0.0f, 0.0f, -5.0f });
        camera.setViewTarget({ 0.0f, 0.0f, 0.0f});
        camera.setPerspectiveProjection(glm::radians(50.0f), 0.1f, 100.0f);
        addChild(camera);

        z0::DirectionalLight directionalLight{
            glm::vec3{-1.0f, .5f, 1.0f},
            glm::vec4{1.0f, .0f, .0f, 1.0f},
            1.0f,
            "Sun"
        };
        addChild(directionalLight);

        //std::shared_ptr<Node> model = z0::Loader::loadModelFromFile("models/free_1972_datsun_240k_gt.glb", false);
        model1 = z0::Loader::loadModelFromFile("models/cube2.glb", true);
        addChild(model1);

        model1->setScale(glm::vec3{ .5 });
        model2 = model1->duplicate();
        addChild(model2);

        model1->setRotationDegrees({ 180.0-16.0,  -25.0, 10.0 });
        model3 = model1->duplicate();
        addChild(model3);

        model1->setPosition(glm::vec3{ -1.5, 0, .0 });
        model2->setPosition(glm::vec3{ .0, -2.0, 4.0 });
        model2->setRotationDegrees({ 180.0-16.0,  25.0, -10.0 });
        model2->setRotationDegrees({ 180.0-16.0,  -25.0, 10.0 });
        model3->setPosition(glm::vec3{ -0, 0, .0 });

        printTree(std::cout);
    }

    void onProcess(float delta) override {
        float angle = delta * glm::radians(90.0f) / 2;
        model1->setRotationX(angle);
        model1->rotateY(angle);
        model2->setRotationX(angle);
        model2->setRotationZ(angle);
    }

private:
    std::shared_ptr<z0::Node> model1;
    std::shared_ptr<z0::Node> model2;
    std::shared_ptr<z0::Node> model3;
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

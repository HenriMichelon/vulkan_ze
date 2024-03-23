#include "z0/mainloop.hpp"
#include "z0/scene.hpp"
#include "z0/loader.hpp"
#include "z0/input.hpp"
#include "z0/nodes/directional_light.hpp"
#include "z0/nodes/camera.hpp"
#include "z0/nodes/environment.hpp"
#include "z0/nodes/omni_light.hpp"
#include "z0/nodes/spot_light.hpp"
#include "z0/nodes/mesh_instance.hpp"
#include "z0/nodes/skybox.hpp"
#include "z0/log.hpp"

#include <algorithm>

void printPosition(z0::Node node) {
    auto pos = node.getPosition();
    std::cout << node.toString() << " local position : " << pos.x << "," << pos.y << "," << pos.z << std::endl;
    pos = node.getPositionGlobal();
    std::cout << node.toString() << " global position : " << pos.x << "," << pos.y << "," << pos.z << std::endl;
}

class Player: public z0::Node {
public:
    const float translationSpeed = 4.0;
    const float mouseSensitivity = 0.002;
    const float maxCameraAngleUp = glm::radians(60.0);
    const float maxCameraAngleDown = -glm::radians(45.0);

    Player(): z0::Node("Player") {}

    void onInput(z0::InputEvent& event) {
        if ((event.getType() == z0::INPUT_EVENT_MOUSE_MOTION) && mouseCaptured) {
            auto& eventMouseMotion = dynamic_cast<z0::InputEventMouseMotion&>(event);
            rotateY(eventMouseMotion.getRelativeX() * mouseSensitivity);
            camera->rotateX(eventMouseMotion.getRelativeY() * mouseSensitivity * mouseInvertedAxisY);
            camera->setRotationX(std::clamp(camera->getRotationX(), maxCameraAngleDown, maxCameraAngleUp));
        } else if (event.getType() == z0::INPUT_EVENT_KEY) {
            auto& eventKey = dynamic_cast<z0::InputEventKey&>(event);
            if ((eventKey.getKeyCode() == z0::KEY_ESCAPE) && eventKey.isPressed()) {
                releaseMouse();
            }
        }
    }

    void onProcess(float delta) override {
        glm::vec2 input;
        if (gamepad != -1) {
            input = z0::Input::getGamepadVector(gamepad, z0::GAMEPAD_AXIS_LEFT);
            if (input == vec2Zero) input = z0::Input::getKeyboardVector(z0::KEY_A, z0::KEY_D, z0::KEY_W, z0::KEY_S);
        } else {
            input = z0::Input::getKeyboardVector(z0::KEY_A, z0::KEY_D, z0::KEY_W, z0::KEY_S);
        }
        if (input != vec2Zero) {
            auto direction = transformBasis * glm::vec3{input.x, 0, -input.y};
            glm::vec3 velocity{direction.x * translationSpeed, 0.0, direction.z * translationSpeed};
            velocity = velocity * delta;
            if (velocity != vec3Zero) {
                captureMouse();
                translate(velocity);
            }
        }
        if (mouseCaptured) {
            glm::vec2 inputDir;
            if (gamepad != -1) {
                inputDir = z0::Input::getGamepadVector(gamepad, z0::GAMEPAD_AXIS_RIGHT);
                if (inputDir == vec2Zero) inputDir = z0::Input::getKeyboardVector(z0::KEY_LEFT, z0::KEY_RIGHT, z0::KEY_UP, z0::KEY_DOWN);
            } else {
                inputDir = z0::Input::getKeyboardVector(z0::KEY_LEFT, z0::KEY_RIGHT, z0::KEY_UP, z0::KEY_DOWN);
            }
            if (inputDir != vec2Zero) {
                auto look_dir = inputDir * delta;
                rotateY(look_dir.x * 2.0);
                camera->rotateX(-look_dir.y * mouseInvertedAxisY);
                camera->setRotationX(std::clamp(camera->getRotationX() , maxCameraAngleDown, maxCameraAngleUp));
            }
        }
    }

    void onReady() override {
        captureMouse();
        setPosition({0.0, 0.0, -10.0});

        auto markup = z0::Loader::loadModelFromFile("models/light.glb", true);
        markup->setScale(glm::vec3{0.25});
        addChild(markup);

        camera = std::make_shared<z0::Camera>();
        camera->setPosition({ 0.0f, 0.0f, -0.5f});
        addChild(static_cast<std::shared_ptr<z0::Node>>(camera));

        for (int i = 0; i < z0::Input::getConnectedJoypads(); i++) {
            if (z0::Input::isGamepad(i)) {
                gamepad = i;
                break;
            }
        }
        if (gamepad != -1) {
            std::cout << "Using Gamepad " << z0::Input::getGamepadName(gamepad) << std::endl;
        }
    }

private:
    int gamepad{-1};
    bool mouseCaptured{false};
    int mouseInvertedAxisY{-1};
    std::shared_ptr<z0::Camera> camera;
    std::shared_ptr<z0::Node> markup2;

    void captureMouse() {
        z0::Input::setMouseMode(z0::MOUSE_MODE_HIDDEN_CAPTURED);
        mouseCaptured = true;
    }

    void releaseMouse() {
        z0::Input::setMouseMode(z0::MOUSE_MODE_VISIBLE);
        mouseCaptured = false;
    }
};

class RootNode: public z0::Node {
public:

    RootNode(): z0::Node("Main") {}

    void onProcess(float delta) override {
        float angle = delta * glm::radians(90.0f) / 2;
        model1->rotateGlobalZ(angle);
        model2->rotateX(angle);
        model3->rotateY(angle);
    }

    void onReady() override {
        z0::Environment environment{};
        environment.setAmbientColorAndIntensity({1.0f, 1.0f, 1.0f, 0.2f});
        addChild(environment);

        z0::Skybox skybox("textures/sky", ".jpg");
        addChild(skybox);

        z0::DirectionalLight directionalLight{glm::vec3{0.0f, .5f, 0.1f}};
        directionalLight.setColorAndIntensity({1.0f, 1.0f, 1.0f, 1.0f});
        directionalLight.setCastShadow(true);
        addChild(directionalLight);

       /* z0::SpotLight spotLight1{{-0.75, .5, 0.1},
                                 45.0, 55.0,
                                 0.027, 0.0028};
        spotLight1.setPosition({3.0, -6.0, -2.1});
        spotLight1.setColorAndIntensity({1.0f, 1.0f, 1.0f, 4.0f});
        spotLight1.setCastShadow(true);
        addChild(spotLight1);
        light1 = z0::Loader::loadModelFromFile("models/light.glb", false);
        light1->setPosition(spotLight1.getPosition());
        addChild(light1);

        z0::SpotLight spotLight2{{0.75, .5, 0.1},
                                 45.0, 55.0,
                                 0.027, 0.0028};
        spotLight2.setPosition({-3.0, -4.0, -2.1});
        spotLight2.setColorAndIntensity({1.0f, 1.0f, 1.0f, 4.0f});
        //spotLight2.setCastShadow(true);
        //addChild(spotLight2);
        light2 =  z0::Loader::loadModelFromFile("models/light.glb", false);
        light2->setPosition(spotLight2.getPosition());
        addChild(light2);*/

        model3 = z0::Loader::loadModelFromFile("models/window.glb");
        model3->rotateDegrees({-90.0, 0.0, 0.0});
        model3->setPosition({1.0, -0.0, -3.0});
        auto* mi = dynamic_cast<z0::MeshInstance*>(model3->getChildren().front().get());
        auto* mat = dynamic_cast<z0::StandardMaterial*>(mi->getMesh()->getSurfaceMaterial(0).get());
        mat->transparency = z0::TRANSPARENCY_ALPHA;
        mat->cullMode = z0::CULLMODE_DISABLED;
        //mat->alphaScissor = 0.4;

        model4 = model3->duplicate();
        model4->setPosition({0.0, -0.0, 4.0});

        addChild(model4);
        addChild(model3);

        model1 = z0::Loader::loadModelFromFile("models/cube2.glb", true);

        model2 = model1->duplicate();
        addChild(model2);
        addChild(model1);

        model1->setScale(glm::vec3{.5});

        model1->setPositionGlobal({4.0, 0.0, 0.0});
        model2->setPosition({0.0, -2.0, 0.0});

        /*floor = z0::Loader::loadModelFromFile("models/floor.glb", true );
        floor->setPosition({0.0, 2.0, 0.0});
        addChild(floor);*/

        addChild(std::make_shared<Player>());
        //printTree(std::cout);
    }

private:
    float rot = 0.0;
    std::shared_ptr<z0::Node> model1;
    std::shared_ptr<z0::Node> model2;
    std::shared_ptr<z0::Node> model3;
    std::shared_ptr<z0::Node> model4;
    std::shared_ptr<z0::Node> light1;
    std::shared_ptr<z0::Node> light2;
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

#include "z0/application.hpp"
#include "z0/loader.hpp"
#include "z0/input.hpp"
#include "z0/nodes/directional_light.hpp"
#include "z0/nodes/camera.hpp"
#include "z0/nodes/environment.hpp"
#include "z0/nodes/omni_light.hpp"
#include "z0/nodes/spot_light.hpp"
#include "z0/nodes/skybox.hpp"
#include "z0/log.hpp"

#include <algorithm>

void printPosition(const z0::Node& node) {
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

    void onInput(z0::InputEvent& event) override {
        if ((event.getType() == z0::INPUT_EVENT_MOUSE_MOTION) && mouseCaptured) {
            auto& eventMouseMotion = dynamic_cast<z0::InputEventMouseMotion&>(event);
            rotateY(-eventMouseMotion.getRelativeX() * mouseSensitivity);
            camera->rotateX(eventMouseMotion.getRelativeY() * mouseSensitivity * mouseInvertedAxisY);
            camera->setRotationX(std::clamp(camera->getRotationX(), maxCameraAngleDown, maxCameraAngleUp));
        }
        if ((event.getType() == z0::INPUT_EVENT_KEY) && mouseCaptured) {
            auto& eventKey = dynamic_cast<z0::InputEventKey&>(event);
            if ((eventKey.getKeyCode() == z0::KEY_ESCAPE) && !eventKey.isPressed()) {
                releaseMouse();
            }
        }
    }

    void onProcess(float delta) override {
        glm::vec2 input;
        if (gamepad != -1) {
            input = z0::Input::getGamepadVector(gamepad, z0::GAMEPAD_AXIS_LEFT);
            if (input == z0::VEC2ZERO) input = z0::Input::getKeyboardVector(z0::KEY_A, z0::KEY_D, z0::KEY_W, z0::KEY_S);
        } else {
            input = z0::Input::getKeyboardVector(z0::KEY_A, z0::KEY_D, z0::KEY_W, z0::KEY_S);
        }
        glm::vec3 velocity{};
        if (input != z0::VEC2ZERO) {
            auto direction = transformBasis * glm::vec3{input.x, 0, input.y};
            velocity.x = direction.x * translationSpeed;
            velocity.z = direction.z * translationSpeed;
        }
        if (z0::Input::isKeyPressed(z0::KEY_Q)) {
            velocity.y += translationSpeed / 2;
        } else if (z0::Input::isKeyPressed(z0::KEY_Z)) {
            velocity.y -= translationSpeed / 2;
        }
        if (velocity != z0::VEC3ZERO) {
            velocity = velocity * delta;
            captureMouse();
            translate(velocity);
        }
        if (mouseCaptured) {
            glm::vec2 inputDir;
            if (gamepad != -1) {
                inputDir = z0::Input::getGamepadVector(gamepad, z0::GAMEPAD_AXIS_RIGHT);
                if (inputDir == z0::VEC2ZERO) inputDir = z0::Input::getKeyboardVector(z0::KEY_LEFT, z0::KEY_RIGHT, z0::KEY_UP, z0::KEY_DOWN);
            } else {
                inputDir = z0::Input::getKeyboardVector(z0::KEY_LEFT, z0::KEY_RIGHT, z0::KEY_UP, z0::KEY_DOWN);
            }
            if (inputDir != z0::VEC2ZERO) {
                auto look_dir = inputDir * delta;
                rotateY(-look_dir.x * 2.0f);
                camera->rotateX(-look_dir.y * mouseInvertedAxisY);
                camera->setRotationX(std::clamp(camera->getRotationX() , maxCameraAngleDown, maxCameraAngleUp));
            }
        }
    }

    void onReady() override {
        captureMouse();
        setPosition({0.0, 0.5, 2.0});
        //rotateY(glm::radians(-45.));

        /*auto markup = z0::Loader::loadModelFromFile("models/light.glb", true);
        markup->setScale(glm::vec3{0.25});
        addChild(markup);*/

        camera = std::make_shared<z0::Camera>();
        camera->setPosition({ 0.0f, 0.0f, 0.5f});
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
        //setProcessMode(z0::PROCESS_MODE_ALWAYS);
    }

private:
    int gamepad{-1};
    bool mouseCaptured{false};
    float mouseInvertedAxisY{1.0};
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
        model1->rotateY(angle);
        model1->rotateX(angle);
    }

    void onReady() override {
        z0::Environment environment{};
        environment.setAmbientColorAndIntensity({1.0f, 1.0f, 1.0f, 0.05f});
        addChild(environment);

        z0::Skybox skybox("textures/sky", ".jpg");
        addChild(skybox);

        z0::DirectionalLight directionalLight{glm::vec3{0.0f, -1.0f, -1.0f}};
        directionalLight.setColorAndIntensity({1.0f, 1.0f, 1.0f, 0.5f});
        directionalLight.setCastShadow(true);
        addChild(directionalLight);

        z0::SpotLight spotLight1{{-.25, -1.25, 1.0},
                                 40.0, 45.0,
                                 0.027, 0.0028};
        spotLight1.setPosition({.2, 2.0, -1.5});
        spotLight1.setColorAndIntensity({1.0f, 1.0f, 1.0f, 1.0f});
        spotLight1.setCastShadow(false);
        addChild(spotLight1);
        light1 = z0::Loader::loadModelFromFile("models/light.glb", true);
        light1->setScale(glm::vec3{0.25});
        light1->setPosition(spotLight1.getPosition());
        addChild(light1);

        addChild(spotLight1.duplicate());
        addChild(spotLight1.duplicate());
        addChild(spotLight1.duplicate());
        addChild(spotLight1.duplicate());
        addChild(spotLight1.duplicate());

        model1 = z0::Loader::loadModelFromFile("models/sphere.glb", false);
        //model1->setScale(glm::vec3{0.01});
        //model1->rotateZ(glm::radians(10.0));
        addChild(model1);

        floor = z0::Loader::loadModelFromFile("models/floor.glb", true);
        floor->setPosition({0.0, -2.0, 0.0});
        addChild(floor);

        addChild(std::make_shared<Player>());
        //printTree(std::cout);
        //z0::Application::setPaused(true);
    }

private:
    float rot = 0.0;
    std::shared_ptr<z0::Node> model1;
    std::shared_ptr<z0::Node> light1;
    std::shared_ptr<z0::Node> floor;

};

int main() {
    z0::ApplicationConfig applicationConfig {
        .appName = "Example App",
        .appDir = "..",
        .windowMode = z0::WINDOW_MODE_WINDOWED,
        .windowWidth = 1024,
        .windowHeight = 768,
        .msaa = z0::MSAA_AUTO,
        .gamma = 1.0f,
    };
    z0::Application app{applicationConfig};
    app.start(std::make_shared<RootNode>());
    return 0;
}

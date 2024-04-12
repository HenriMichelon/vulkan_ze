#include "z0/application.hpp"
#include "z0/loader.hpp"
#include "z0/input.hpp"
#include "z0/nodes/directional_light.hpp"
#include "z0/nodes/camera.hpp"
#include "z0/nodes/environment.hpp"
#include "z0/nodes/omni_light.hpp"
#include "z0/nodes/spot_light.hpp"
#include "z0/nodes/skybox.hpp"
#include "z0/nodes/rigid_body.hpp"
#include "z0/nodes/static_body.hpp"
#include "z0/nodes/mesh_instance.hpp"
#include "z0/log.hpp"
#include "z0/nodes/multi_mesh_instance.hpp"

#include <algorithm>
#include <glm/gtc/quaternion.hpp>
#include <utility>

enum Layers {
    WORLD       = 0b0001,
    BODIES      = 0b0010,
};

class Player: public z0::Node {
public:
    const float translationSpeed = 4;
    const float mouseSensitivity = 0.002;
    const float viewSensitivity = 0.1;
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

    void onPhysicsProcess(float delta) override {
        previousState = currentState;
        glm::vec2 input;
        if (gamepad != -1) {
            input = z0::Input::getGamepadVector(gamepad, z0::GAMEPAD_AXIS_LEFT);
            if (input == z0::VEC2ZERO) input = z0::Input::getKeyboardVector(z0::KEY_A, z0::KEY_D, z0::KEY_W, z0::KEY_S);
        } else {
            input = z0::Input::getKeyboardVector(z0::KEY_A, z0::KEY_D, z0::KEY_W, z0::KEY_S);
        }

        currentState = State{};
        if (input != z0::VEC2ZERO) {
            auto direction = transformBasis * glm::vec3{input.x, 0, input.y};
            currentState.velocity.x = direction.x * translationSpeed;
            currentState.velocity.z = direction.z * translationSpeed;
        }
        if (z0::Input::isKeyPressed(z0::KEY_Q)) {
            currentState.velocity.y += translationSpeed / 2;
        } else if (z0::Input::isKeyPressed(z0::KEY_Z)) {
            currentState.velocity.y -= translationSpeed / 2;
        }
        if (currentState.velocity != z0::VEC3ZERO) currentState.velocity *= delta;

        if (mouseCaptured) {
            glm::vec2 inputDir;
            if (gamepad != -1) {
                inputDir = z0::Input::getGamepadVector(gamepad, z0::GAMEPAD_AXIS_RIGHT);
                if (inputDir == z0::VEC2ZERO) inputDir = z0::Input::getKeyboardVector(z0::KEY_LEFT, z0::KEY_RIGHT, z0::KEY_UP, z0::KEY_DOWN);
            } else {
                inputDir = z0::Input::getKeyboardVector(z0::KEY_LEFT, z0::KEY_RIGHT, z0::KEY_UP, z0::KEY_DOWN);
            }
            if (inputDir != z0::VEC2ZERO) currentState.lookDir = inputDir * viewSensitivity * delta;
        }
    }

    void onProcess(float alpha) override {
        if (currentState.velocity != z0::VEC3ZERO) {
            captureMouse();
            auto interpolatedVelocity = previousState.velocity * (1.0f-alpha) + currentState.velocity * alpha;
            translate(interpolatedVelocity);
        }
        if (currentState.lookDir != z0::VEC2ZERO) {
            captureMouse();
            auto interpolatedLookDir = previousState.lookDir * (1.0f-alpha) + currentState.lookDir * alpha;
            rotateY(-interpolatedLookDir.x * 2.0f);
            camera->rotateX(interpolatedLookDir.y * keyboardInvertedAxisY);
            camera->setRotationX(std::clamp(camera->getRotationX() , maxCameraAngleDown, maxCameraAngleUp));
        }
    }

    void onReady() override {
        captureMouse();
        setPosition({0.0, 1.5, 10.0});

        camera = std::make_shared<z0::Camera>();
        camera->setPosition({ 0.0f, 0.0f, 0.0f});
        addChild(camera);

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
    struct State {
        glm::vec3 velocity = z0::VEC3ZERO;
        glm::vec2 lookDir = z0::VEC2ZERO;
        State& operator=(const State& other) = default;
    };

    int gamepad{-1};
    bool mouseCaptured{false};
    float mouseInvertedAxisY{1.0};
    float keyboardInvertedAxisY{1.0};
    State previousState;
    State currentState;
    std::shared_ptr<z0::Camera> camera;

    void captureMouse() {
        z0::Input::setMouseMode(z0::MOUSE_MODE_HIDDEN_CAPTURED);
        mouseCaptured = true;
    }

    void releaseMouse() {
        z0::Input::setMouseMode(z0::MOUSE_MODE_VISIBLE);
        mouseCaptured = false;
    }
};


class Crate: public z0::RigidBody {
public:
    explicit Crate(std::shared_ptr<z0::Node> model):
        z0::RigidBody{std::make_shared<z0::BoxShape>(glm::vec3{2.0f,2.0f, 2.0f}),
                        Layers::BODIES,
                        Layers::WORLD | Layers::BODIES} {
        addChild(std::move(model));
        setBounce(0.8);
        setGravityScale(0.5);
    }

    void onReady() override {
        glm::quat rot = glm::angleAxis(glm::radians(static_cast<float>(std::rand()%90)), z0::AXIS_Z);
        setRotation(rot);
    }

};

class RootNode: public z0::Node {
public:

    RootNode(): z0::Node("Main") {}

    void onReady() override {
        std::shared_ptr<z0::Environment> environment = std::make_shared<z0::Environment>();
        environment->setAmbientColorAndIntensity({1.0f, 1.0f, 1.0f, 0.05f});
        addChild(environment);

        std::shared_ptr<z0::Skybox> skybox = std::make_shared<z0::Skybox>("textures/sky", ".jpg");
        addChild(skybox);

        std::shared_ptr<z0::DirectionalLight> directionalLight = std::make_shared<z0::DirectionalLight>(glm::vec3{0.0f, -1.0f, -1.0f});
        directionalLight->setColorAndIntensity({1.0f, 1.0f, 1.0f, 0.5f});
        directionalLight->setCastShadow(true);
        addChild(directionalLight);

        auto crateModel = z0::Loader::loadModelFromFile("models/crate.glb", true);
        for (int x = 0; x < 10; x++) {
            for (int z = 0; z < 10; z++) {
                auto model= std::make_shared<Crate>(crateModel->duplicate());
                model->setPosition({x * 3 - 2*10, 3.0 + std::rand() % 5, -z * 3});
                //addChild(model);
            }
        }
        /*std::shared_ptr<z0::MeshInstance> meshInstance = std::static_pointer_cast<z0::MeshInstance>(crateModel->getNode(
                "Sketchfab_model/Collada visual scene group/g/defaultMaterial"));
        auto mesh = meshInstance->getMesh();
        std::shared_ptr<z0::MultiMeshInstance> multi = std::make_shared<z0::MultiMeshInstance>(
                mesh,
                10);
        addChild(multi);*/

        auto floor = std::make_shared<z0::StaticBody>(std::make_shared<z0::BoxShape>(glm::vec3{200.0f,0.2f, 200.0f}),
                                                 Layers::WORLD,
                                                 0);
        floor->addChild(z0::Loader::loadModelFromFile("models/floor.glb", true));
        floor->setPosition({0.0, -2.0, 0.0});
        addChild(floor);

        addChild(std::make_shared<Player>());
        //printTree(std::cout);
    }

private:
    float rot = 0.0;
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

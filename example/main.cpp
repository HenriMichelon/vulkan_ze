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

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>


namespace Layers
{
    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer MOVING = 1;
    static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
};

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
    {
        switch (inObject1)
        {
            case Layers::NON_MOVING:
                return inObject2 == Layers::MOVING; // Non moving only collides with moving
            case Layers::MOVING:
                return true; // Moving collides with everything
            default:
                JPH_ASSERT(false);
                return false;
        }
    }
};

// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
namespace BroadPhaseLayers
{
    static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
    static constexpr JPH::BroadPhaseLayer MOVING(1);
    static constexpr uint32_t NUM_LAYERS(2);
};

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
    BPLayerInterfaceImpl()
    {
        // Create a mapping table from object to broad phase layer
        mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
    }

    virtual uint32_t GetNumBroadPhaseLayers() const override
    {
        return BroadPhaseLayers::NUM_LAYERS;
    }

    virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
    {
        JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
        return mObjectToBroadPhase[inLayer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    virtual const char * GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
    {
        switch ((JPH::BroadPhaseLayer::Type)inLayer)
        {
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
            default:													JPH_ASSERT(false); return "INVALID";
        }
    }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
    JPH::BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
    virtual bool				ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
    {
        switch (inLayer1)
        {
            case Layers::NON_MOVING:
                return inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::MOVING:
                return true;
            default:
                JPH_ASSERT(false);
                return false;
        }
    }
};

JPH::PhysicsSystem physicsSystem;
BPLayerInterfaceImpl broad_phase_layer_interface;
ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
ObjectLayerPairFilterImpl object_vs_object_layer_filter;
std::unique_ptr<JPH::TempAllocatorImpl> temp_allocator;
std::unique_ptr<JPH::JobSystemThreadPool> job_system;

void printPosition(const z0::Node& node) {
    auto pos = node.getPosition();
    std::cout << node.toString() << " local position : " << pos.x << "," << pos.y << "," << pos.z << std::endl;
    pos = node.getPositionGlobal();
    std::cout << node.toString() << " global position : " << pos.x << "," << pos.y << "," << pos.z << std::endl;
}

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
        setPosition({0.0, 2.0, 6.0});
        //rotateY(glm::radians(-45.));

        /*auto markup = z0::Loader::loadModelFromFile("models/light.glb", true);
        markup->setScale(glm::vec3{0.25});
        addChild(markup);*/

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
        //setProcessMode(z0::PROCESS_MODE_ALWAYS);
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


class Crate: public z0::Node {
public:
    explicit Crate(std::shared_ptr<z0::Node> _model): model{_model} {
        addChild(model);
    }

    void onPhysicsProcess(float delta) override {
        float angle = delta * glm::radians(90.0f) / 2;
        model->rotateX(angle);
        model->rotateY(angle);
    }

private:
    std::shared_ptr<z0::Node> model;
};

class RootNode: public z0::Node {
public:

    RootNode(): z0::Node("Main") {}

    void onPhysicsProcess(float delta) override {
        float angle = delta * glm::radians(90.0f) / 2;
        //model1->translate({1.5*delta, 0.0, 0.0});
        /*JPH::BodyInterface &body_interface = physicsSystem.GetBodyInterface();
        JPH::RVec3 position = body_interface.GetPosition(box_id);
        model1->setPosition({position.GetX(), position.GetY(), position.GetZ()});*/
        physicsSystem.Update(delta, 1, temp_allocator.get(), job_system.get());
    }

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

        /*std::shared_ptr<z0::SpotLight> spotLight1 = std::make_shared<z0::SpotLight>(
                glm::vec3{-.25, -1.25, 1.0},
                 40.0, 45.0,
                 0.027, 0.0028);
        spotLight1->setPosition({.2, 2.0, -1.5});
        spotLight1->setColorAndIntensity({1.0f, 1.0f, 1.0f, 1.0f});
        spotLight1->setCastShadow(false);
        addChild(spotLight1);
        light1 = z0::Loader::loadModelFromFile("models/light.glb", true);
        light1->setScale(glm::vec3{0.25});
        light1->setPosition(spotLight1->getPosition());
        addChild(light1);*/

        model1 = z0::Loader::loadModelFromFile("models/crate.glb", false);
        for (int x = 0; x < 2; x++) {
            for (int y = 0; y < 2; y++) {
                for (int z = 0; z < 2; z++) {
                    auto model = std::make_shared<Crate>(model1->duplicate());
                    model->setPosition({x * 4 - 2 * 2, y * 4, z * -4});
                    addChild(model);
                }
            }
        }

        JPH::BodyInterface &body_interface = physicsSystem.GetBodyInterface();

        /*
        model1->setPosition({-3.0, 0.0, 0.0});
        addChild(model1);


        JPH::BodyCreationSettings box_settings(new JPH::BoxShape(JPH::Vec3(1.0f, 1.0f, 1.0f)),
                                               JPH::RVec3(0.0, 2.0, 0.0),
                                               JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic,
                                               Layers::MOVING);
        box_id = body_interface.CreateAndAddBody(box_settings, JPH::EActivation::Activate);
        //body_interface.SetLinearVelocity(box_id, JPH::Vec3(0.0f, -1.0f, 0.0f));
        auto position = body_interface.GetPosition(box_id);
        body_interface.SetRestitution(box_id, 0.8);
        std::cout << position.GetX() << " " << position.GetY() << " " << position.GetZ() << std::endl;
        model1->setPosition({position.GetX(), position.GetY(), position.GetZ()});*/
        //body_interface.SetPosition(box_id, JPH::RVec3(model1->getPositionGlobal().x, model1->getPositionGlobal().y, model1->getPositionGlobal().z), JPH::EActivation::Activate );

        //model2 = model1->duplicate();
        //model2->setPosition({1.0, 0.0, 0.0});
        //addChild(model2);

        JPH::BodyCreationSettings floor_settings(new JPH::BoxShape(JPH::Vec3(100.0f, 0.1f, 100.0f)),
                                                 JPH::RVec3(0.0, -2.0, 0.0),
                                                 JPH::Quat::sIdentity(),
                                                 JPH::EMotionType::Static, Layers::NON_MOVING);
        JPH::BodyID floor_id = body_interface.CreateAndAddBody(floor_settings, JPH::EActivation::DontActivate);

        floor = z0::Loader::loadModelFromFile("models/floor.glb", true);
        //floor->setPosition({0.0, -2.0, 0.0});
        auto position = body_interface.GetPosition(floor_id);
        floor->setPosition({position.GetX(), position.GetY(), position.GetZ()});
        addChild(floor);

        addChild(std::make_shared<Player>());
        //z0::Application::setPaused(true);

        //auto child = getNode("Player/Camera");
        //printTree(std::cout);

        physicsSystem.OptimizeBroadPhase();
    }

private:
    float rot = 0.0;
    std::shared_ptr<z0::Node> model1;
    std::shared_ptr<z0::Node> model2;
    std::shared_ptr<z0::Node> light1;
    std::shared_ptr<z0::Node> floor;

    JPH::BodyID box_id;

};

int main() {
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();
    temp_allocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
    job_system = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, JPH::thread::hardware_concurrency() - 1);
    const uint32_t cMaxBodies = 1024;
    const uint32_t cNumBodyMutexes = 0;
    const uint32_t cMaxBodyPairs = 1024;
    const uint32_t cMaxContactConstraints = 1024;
    physicsSystem.Init(cMaxBodies,
                        cNumBodyMutexes,
                        cMaxBodyPairs,
                        cMaxContactConstraints,
                        broad_phase_layer_interface,
                        object_vs_broadphase_layer_filter,
                        object_vs_object_layer_filter);


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
